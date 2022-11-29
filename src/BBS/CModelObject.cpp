#include "CModelObject.h"
#include <algorithm>
#include <numeric>
#include "Utils\BitCast.h"

using namespace BBS;

CModelObject::CModelObject()
	: boundingBox(glm::vec3(0.0f), glm::vec3(0.0f))
{
	scale = 0.0f;
	for (int i = 0; i < 8; i++) bbox[i] = glm::vec4(0.0f);
	ownsTextures = false;
	mesh0 = nullptr;
	mesh1 = nullptr;
}

CModelObject::~CModelObject()
{
	if (mesh0)
	{
		delete mesh0;
		mesh0 = nullptr;
	}

	if (mesh1)
	{
		delete mesh1;
		mesh1 = nullptr;
	}

	if (ownsTextures)
	{
		for (auto texture : textureObjects)
			delete texture;
	}
}

void CModelObject::LoadPmo(PmoFile& pmo, bool loadTextures)
{
	scale = pmo.header.scale;

	// read bounding box
	for (int i = 0; i < 8; i++)
	{
		bbox[i] = glm::vec4(pmo.header.boundingBox[(i * 4) + 0],
			pmo.header.boundingBox[(i * 4) + 1],
			pmo.header.boundingBox[(i * 4) + 2],
			pmo.header.boundingBox[(i * 4) + 3]);
	}

	glm::vec4 min = glm::vec4(FLT_MAX);
	glm::vec4 max = glm::vec4(-FLT_MAX);
	for (auto& vert : bbox)
	{
		min = glm::min(min, vert);
		max = glm::max(max, vert);
	}

	boundingBox = AABBox(glm::vec3(min), glm::vec3(max));

	if (pmo.hasMesh0())
	{
		sections.reserve(pmo.mesh0.size());
		for (PmoMesh& mesh : pmo.mesh0)
		{
			if (mesh.header.vertexCount == 0) continue;
			CModelSection* section = new CModelSection();
			section->LoadSection(mesh);
			sections.push_back(section);
		}
	}

	if (pmo.hasMesh1())
	{
		transSections.reserve(pmo.mesh1.size());
		for (PmoMesh& mesh : pmo.mesh1)
		{
			if (mesh.header.vertexCount == 0) continue;
			CModelSection* section = new CModelSection();
			section->LoadSection(mesh);
			transSections.push_back(section);
		}
	}

	ownsTextures = loadTextures;
	textureNames.reserve(pmo.textures.size());
	for (PmoTexture& tex : pmo.textures)
	{
		this->textureNames.push_back(std::string(tex.resourceName, 12));
		//if (loadTextures) LoadTexture(tex);
	}
}

void CModelObject::LinkExtTextures(std::unordered_map<std::string, CTextureInfo*> textureMap)
{
	textureObjects.reserve(textureNames.size());
	for (std::string& texName : textureNames)
	{
		CTextureInfo* texObj = textureMap.at(texName);
		textureObjects.push_back(texObj);
	}
}

void CModelObject::UpdateTextureOffsets()
{
	// TODO: we could store an appropriatly sized vector on the model and just update it
	// once per frame. Or we could drop the scrolls right into the mesh's vectors and skip
	// the multiple implicit loops in the copy operators.
	std::vector<glm::vec2> texOffs;
	texOffs.reserve(textureObjects.size());
	for (auto texture : textureObjects)
	{
		texOffs.push_back(texture->currentScroll);
	}
	if (mesh0) mesh0->uvOffsets = texOffs;
	if (mesh1) mesh1->uvOffsets = texOffs;
}

CMesh* CModelObject::BuildMesh(std::vector<CModelSection*>& sections)
{
	if (sections.size() == 0) return nullptr;

	CMesh* mesh = new CMesh();

	int totalVcount = 0;
	for (CModelSection* sec : sections)
	{
		totalVcount += sec->vertexCount;
	}
	mesh->vertCount = totalVcount;
	mesh->vertData.reserve(10 * totalVcount);

	for (CModelSection * modelSection : sections)
	{
		CMeshSection meshSection;
		meshSection.vertCount = modelSection->vertexCount;
		// TODO: Dummy (idx: -1) texture
		meshSection.textureIndex = modelSection->textureIndex;
		meshSection.stride = (10 * sizeof(float));
		meshSection.twoSided = modelSection->attributes & ATTR_BACK;
		meshSection.blend = modelSection->attributes & ATTR_BLEND_MASK;
		for (uint16 kick : modelSection->primCount)
		{
			meshSection.kickList.push_back((unsigned int)kick);
		}
		meshSection.primType = modelSection->primativeType;
		mesh->sections.push_back(meshSection);
		int rp = 0;
		for (int v = 0; v < modelSection->vertexCount; v++)
		{
			if (modelSection->flags.hasWeights)
			{
				// TODO
			}

			if (modelSection->flags.hasTex)
			{
				mesh->vertData.push_back(modelSection->vertexData[rp++]);
				mesh->vertData.push_back(modelSection->vertexData[rp++]);
			}
			else
			{
				mesh->vertData.push_back(0.0f);
				mesh->vertData.push_back(0.0f);
			}

			if (modelSection->flags.hasVColor)
			{
				mesh->vertData.push_back(modelSection->vertexData[rp++]);
				mesh->vertData.push_back(modelSection->vertexData[rp++]);
				mesh->vertData.push_back(modelSection->vertexData[rp++]);
				mesh->vertData.push_back(modelSection->vertexData[rp++]);
			}
			else if (modelSection->flags.hasCColor)
			{
				mesh->vertData.push_back((float)((modelSection->globalColor & 0x000000FF) >> 0) / 127.f);
				mesh->vertData.push_back((float)((modelSection->globalColor & 0x0000FF00) >> 8) / 127.f);
				mesh->vertData.push_back((float)((modelSection->globalColor & 0x00FF0000) >> 16) / 127.f);
				mesh->vertData.push_back((float)((modelSection->globalColor & 0xFF000000) >> 24) / 255.f);
			}
			else
			{
				mesh->vertData.push_back((float)0x80 / 127.f);
				mesh->vertData.push_back((float)0x80 / 127.f);
				mesh->vertData.push_back((float)0x80 / 127.f);
				mesh->vertData.push_back((float)0xFF / 255.f);
			}

			if (modelSection->flags.hasPosition)
			{
				mesh->vertData.push_back(modelSection->vertexData[rp++] * this->scale);
				mesh->vertData.push_back(modelSection->vertexData[rp++] * this->scale);
				mesh->vertData.push_back(modelSection->vertexData[rp++] * this->scale);
				mesh->vertData.push_back(1.0f);
			}
			else
			{
				mesh->vertData.push_back(0.0f);
				mesh->vertData.push_back(0.0f);
				mesh->vertData.push_back(0.0f);
				mesh->vertData.push_back(1.0f);
			}
		}

		mesh->polyCount += std::accumulate(modelSection->primCount.begin(), modelSection->primCount.end(), 0);
	}
	mesh->Build();
	return mesh;
}

void CModelObject::BuildMesh()
{
	mesh0 = BuildMesh(sections);
	mesh1 = BuildMesh(transSections);

	if (mesh0 == nullptr && mesh1 == nullptr) return;

	std::vector<CTexture*> textureList;
	textureList.reserve(textureObjects.size());
	for (auto texEntry : this->textureObjects)
		textureList.push_back(texEntry->srcTexture->texture);
	if (mesh0 != nullptr) mesh0->textures = textureList;
	if (mesh1 != nullptr) mesh1->textures = textureList;
}

void CModelObject::DoDraw(RenderContext& render, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale)
{
	ERenderLayer pass = render.render.currentPass;
	if (pass == LAYER_SKY)
	{
		if (mesh0) mesh0->Draw(render, pos, rot, scale);
		if (mesh1) mesh1->Draw(render, pos, rot, scale);
	}
	else if (pass == LAYER_STATIC)
	{
		if (mesh0) mesh0->Draw(render, pos, rot, scale);
	}
	else if (pass == LAYER_DYNAMIC)
	{
		if (mesh1) mesh1->Draw(render, pos, rot, scale);
	}
}

VertexFlags BBS::Merge(const VertexFlags& a, const VertexFlags& b)
{
	VertexFlags c{};
	c.hasWeights = (a.hasWeights | b.hasWeights);
	c.hasTex = (a.hasTex | b.hasTex);
	c.hasVColor = (a.hasVColor | b.hasVColor);
	c.hasPosition = (a.hasPosition | b.hasPosition);
	c.hasCColor = (a.hasCColor | b.hasCColor);
	return c;
}

CModelSection::CModelSection()
{
	textureIndex = 0;
	vertexCount = 0;
	primativeType = GL_TRIANGLES;
	flags = { 0 };
	attributes = 0;
	globalColor = 0xFF808080;
}

void CModelSection::LoadSection(PmoMesh& mesh)
{
	uint32 constDiffuse = 0;
	int elementCount = 0;

	PmoVertexFormatFlags format = mesh.header.vertexFormat;

	this->textureIndex = mesh.header.textureIndex;
	this->vertexCount = mesh.header.vertexCount;
	switch (format.primative)
	{
	case 3: this->primativeType = GL_TRIANGLES; break;
	case 4: this->primativeType = GL_TRIANGLE_STRIP; break;
	default:
		break;
	}
	this->attributes = mesh.header.attributes;

	if (format.color != 0)
	{
		this->flags.hasVColor = 1;
		elementCount += 4;
	}
	if (format.texCoord != 0)
	{
		this->flags.hasTex = 1;
		elementCount += 2;
	}
	this->flags.hasPosition = 1;
	elementCount += 3;

	if (format.diffuse != 0)
	{
		this->globalColor = mesh.header.diffuseColor;
		this->flags.hasCColor = 1;
	}

	if (mesh.header.triStripCount > 0)
	{
		this->primCount = mesh.header.triStripLengths;
	}
	else
	{
		primCount.push_back(mesh.header.vertexCount);
	}

	// read verts
	uint8* pData = mesh.vertexData.data();
	vertexData.reserve(mesh.header.vertexCount * elementCount);
	for (int vi = 0; vi < mesh.header.vertexCount; vi++)
	{
		uint8* pVertStart = pData + (vi * mesh.header.vertexSize);
		uint8 readPtr = 0;

		uint8 temp8[4] = { 0 };
		uint16 temp16[4] = { 0 };
		uint32 temp32[4] = { 0 };
		switch (format.weights)
		{
		case 0: // none 
			break;
		case 1: // uint8
			readPtr += format.skinning + 1;
			break;
		default:
			// TODO: Error
			break;
		}

		switch (format.texCoord)
		{
		case 0: // none
			break;
		case 1: // uint8
			temp8[0] = pVertStart[readPtr++];
			temp8[1] = pVertStart[readPtr++];
			vertexData.push_back((float)temp8[0] / 127.0f);
			vertexData.push_back((float)temp8[1] / 127.0f);
			break;
		case 2:	// uint16
			readPtr += ((0x2 - ((readPtr - 0) & 0x1)) & 0x1);
			temp16[0] = (pVertStart[readPtr++] << 0) | (pVertStart[readPtr++] << 8);
			temp16[1] = (pVertStart[readPtr++] << 0) | (pVertStart[readPtr++] << 8);
			vertexData.push_back((float)temp16[0] / 32767.0f);
			vertexData.push_back((float)temp16[1] / 32767.0f);
			break;
		case 3: // float
			readPtr += ((0x4 - ((readPtr - 0) & 0x3)) & 0x3);
			temp32[0] = (pVertStart[readPtr++] << 0) | (pVertStart[readPtr++] << 8)
				| (pVertStart[readPtr++] << 16) | (pVertStart[readPtr++] << 24);
			temp32[1] = (pVertStart[readPtr++] << 0) | (pVertStart[readPtr++] << 8)
				| (pVertStart[readPtr++] << 16) | (pVertStart[readPtr++] << 24);
			vertexData.push_back(bit_cast<float, uint32>(temp32[0]));
			vertexData.push_back(bit_cast<float, uint32>(temp32[1]));
			break;
		}

		switch (format.color)
		{
		case 0: // none
			break;
		case 4: // BRG-5650
			readPtr += ((0x2 - ((readPtr - 0) & 0x1)) & 0x1);
			temp16[0] = (pVertStart[readPtr++] << 0) | (pVertStart[readPtr++] << 8);
			vertexData.push_back(((float)((temp16[0] & 0xF800) >> 11) / 31.f) * 255.f);
			vertexData.push_back(((float)((temp16[0] & 0x07E0) >> 5) / 63.f) * 255.f);
			vertexData.push_back(((float)((temp16[0] & 0x001F) >> 0) / 31.f) * 255.f);
			vertexData.push_back(0xFF);
			break;
		case 5: // ABGR-5551
			readPtr += ((0x2 - ((readPtr - 0) & 0x1)) & 0x1);
			temp16[0] = (pVertStart[readPtr++] << 0) | (pVertStart[readPtr++] << 8);
			vertexData.push_back(((float)((temp16[0] & 0xF800) >> 11) / 31.f) * 255.f);
			vertexData.push_back(((float)((temp16[0] & 0x07C0) >> 6) / 31.f) * 255.f);
			vertexData.push_back(((float)((temp16[0] & 0x003E) >> 1) / 31.f) * 255.f);
			vertexData.push_back((temp16[0] & 0x8000) ? (0xFF) : (0));
			break;
		case 6: // ABGR-4444
			readPtr += ((0x2 - ((readPtr - 0) & 0x1)) & 0x1);
			temp16[0] = (pVertStart[readPtr++] << 0) | (pVertStart[readPtr++] << 8);
			vertexData.push_back(((float)((temp16[0] & 0xF000) >> 12) / 15.f) * 255.f);
			vertexData.push_back(((float)((temp16[0] & 0x0F00) >> 8) / 15.f) * 255.f);
			vertexData.push_back(((float)((temp16[0] & 0x00F0) >> 4) / 15.f) * 255.f);
			vertexData.push_back(((float)((temp16[0] & 0x000F) >> 0) / 15.f) * 255.f);
			break;
		case 7: // ABGR-8888
			readPtr += ((0x4 - ((readPtr - 0) & 0x3)) & 0x3);
			temp8[0] = pVertStart[readPtr++];
			temp8[1] = pVertStart[readPtr++];
			temp8[2] = pVertStart[readPtr++];
			temp8[3] = pVertStart[readPtr++];
			vertexData.push_back((float)temp8[0] / 127.0f);
			vertexData.push_back((float)temp8[1] / 127.0f);
			vertexData.push_back((float)temp8[2] / 127.0f);
			vertexData.push_back((float)temp8[3] / 255.0f);
			break;
		default:
			// TODO: Error
			break;
		}

		switch (format.position)
		{
		case 1:
			temp8[0] = pVertStart[readPtr++];
			temp8[1] = pVertStart[readPtr++];
			temp8[2] = pVertStart[readPtr++];
			vertexData.push_back((float)bit_cast<int8, uint8>(temp8[0]) / 127.0f);
			vertexData.push_back((float)bit_cast<int8, uint8>(temp8[1]) / 127.0f);
			vertexData.push_back((float)bit_cast<int8, uint8>(temp8[2]) / 127.0f);
			break;
		case 2:
			readPtr += ((0x2 - ((readPtr - 0) & 0x1)) & 0x1);
			temp16[0] = (pVertStart[readPtr++] << 0) | (pVertStart[readPtr++] << 8);
			temp16[1] = (pVertStart[readPtr++] << 0) | (pVertStart[readPtr++] << 8);
			temp16[2] = (pVertStart[readPtr++] << 0) | (pVertStart[readPtr++] << 8);
			vertexData.push_back((float)bit_cast<int16, uint16>(temp16[0]) / 32767.0f);
			vertexData.push_back((float)bit_cast<int16, uint16>(temp16[1]) / 32767.0f);
			vertexData.push_back((float)bit_cast<int16, uint16>(temp16[2]) / 32767.0f);
			break;
		case 3:
			readPtr += ((0x4 - ((readPtr - 0) & 0x3)) & 0x3);
			temp32[0] = (pVertStart[readPtr++] << 0) | (pVertStart[readPtr++] << 8)
				| (pVertStart[readPtr++] << 16) | (pVertStart[readPtr++] << 24);
			temp32[1] = (pVertStart[readPtr++] << 0) | (pVertStart[readPtr++] << 8)
				| (pVertStart[readPtr++] << 16) | (pVertStart[readPtr++] << 24);
			temp32[2] = (pVertStart[readPtr++] << 0) | (pVertStart[readPtr++] << 8)
				| (pVertStart[readPtr++] << 16) | (pVertStart[readPtr++] << 24);
			vertexData.push_back(bit_cast<float, uint32>(temp32[0]));
			vertexData.push_back(bit_cast<float, uint32>(temp32[1]));
			vertexData.push_back(bit_cast<float, uint32>(temp32[2]));
		}
	}
}