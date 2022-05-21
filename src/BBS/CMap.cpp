#include "CMap.h"
#include "PMP.h"
#include "PMO.h"
#include "..\Utils\BitCast.h"
#include <fstream>
#include "..\FileTypes\BbsPmp.h"
#include <algorithm>
#include <numeric>
#include "..\Utils\Math.h"
#include <math.h>
#include "..\Core\CCamera.h"

using namespace BBS;

void CMap::LoadMapFile(std::string filePath)
{
	std::ifstream fileStream;
	fileStream.open(filePath, std::fstream::in | std::fstream::binary);
	if (!fileStream.is_open())
	{
		// TODO: error
		return;
	}

	PmpFile pmpFile;
	pmpFile.ReadPmpFile(fileStream);
	std::cout << "[MAP] Loaded PMP: " << filePath << std::endl;

	this->instances.reserve(pmpFile.header.instance_count);
	this->objects.resize(pmpFile.header.model_count);	// Note: resize not reserve because we're going to index into it directly and treat the vector like an array.
	this->textures.reserve(pmpFile.header.tex_count);

	// Create instances, loading the actual models as we go
	for (PmpInstance& inst : pmpFile.instances)
	{
		if (inst.offset != 0)
		{
			if (this->objects[inst.model_idx] == nullptr)
			{
				this->objects[inst.model_idx] = new CModelObject();
				this->objects[inst.model_idx]->LoadPmo(inst.data, false);
			}
		}

		CMapInstance* mapInstance = new CMapInstance(this, inst);
		this->instances.push_back(mapInstance);
	}
	int trueObjectCount = std::count_if(std::begin(objects), std::end(objects), [](CModelObject* obj) { return obj != nullptr; });
	std::cout << "[MAP] Loaded " << trueObjectCount << "/" << objects.size() << " objects and " << instances.size() << " instances" << std::endl;

	// Load textures
	for (PmpTexEntry& tex : pmpFile.textures)
	{
		CTextureObject* texObj = new CTextureObject();
		texObj->LoadTM2(tex.data);
		CTextureInfo* mapTexture = new CTextureInfo(this, tex, texObj);
		this->textures.insert(std::make_pair(mapTexture->name, mapTexture));
	}
	std::cout << "[MAP] Loaded " << textures.size() << " textures" << std::endl;

	// Give the models their textures
	for each (CModelObject * model in this->objects)
	{
		if (model != nullptr)
		{
			model->LinkExtTextures(this->textures);
			model->BuildMesh();
		}
	}
	std::cout << "[MAP] Load Complete" << std::endl;

}

// The CModelObject holds a pointer to the CMesh object and the CTextureObject
// holds a pointer to the CTexture. (CTexture is a direct wrapper around an opengl texture,
// CMesh is a little more complicated but does basically the same thing for VAO/VBO)
// We'll need to be careful linking the CMesh to the CTextures.
// 
// Rendering
//		In Update we perform the frustum cull and enqueue passing instances for render
//		in static + dynamic queues (if the object has a transparent component)
//		
//		Then the renderer calls on each CMapInstance in the appropriate passes.
//		The instance calls into the CModelObject with the transform and the pass.
//		The object gets the texture scroll info from the CTextureObject and the
//		OpenGL data from the CMesh and uses that to render.

void CMap::Update(WorldContext& context)
{
	for (auto& texture : textures)
	{
		texture.second->Update(context.deltaTime, context.worldTime);
	}

	for (auto model : objects)
	{
		model->UpdateTextureOffsets();
	}

	for (auto instance : instances)
	{
		instance->Update(context.deltaTime, context.worldTime);

		if (instance->flags & FLAG_SKYBOX)
		{
			context.render->render.skyDrawList.push_back(instance);
		}
		else
		{
			if (instance->BBox(context.render->render.current_camera, context.render->render.projectionMatrix))
			{
				context.render->render.staticDrawList.push_back(instance);
				if (instance->model->mesh1 != nullptr)
					context.render->render.dynamicDrawList.push_back(instance);
			}
		}
	}

}

void CMap::Clear()
{
	std::cout << "[MAP] Beginning unload" << std::endl;

	for (auto instance : instances)
		delete instance;

	for (auto object : objects)
		delete object;

	for (auto& texture : textures)
		delete texture.second;
}

CMapInstance::CMapInstance(CMap* map, PmpInstance& inst)
{
	this->parent = map;
	this->position = glm::vec3(inst.loc[0], inst.loc[1], inst.loc[2]);
	this->rotation = glm::vec3(inst.rot[0], inst.rot[1], inst.rot[2]);
	this->scale = glm::vec3(inst.scale[0], inst.scale[1], inst.scale[2]);
	this->objectID = inst.model_idx;
	this->flags = inst.flags;

	this->model = this->parent->objects[this->objectID];
}

void CMapInstance::DoDraw(RenderContext& render)
{
	if (model)
		model->DoDraw(render, position, rotation, scale);
}

float CMapInstance::CalcZ(RenderContext& context)
{
	glm::vec4 objCamPos = context.render.viewMatrix * glm::translate(glm::mat4(1.0f), this->position) * glm::vec4(this->position, 1.0f);
	return -objCamPos.z;
}

void CMapInstance::Update(float deltaTime, double worldTime)
{
	// Don't think this needs to do anything?
}

bool CMapInstance::BBox(CCamera* cam, glm::mat4 projMatrix)
{
	glm::vec4 min = glm::vec4(FLT_MAX);
	glm::vec4 max = glm::vec4(-FLT_MAX);
	for (auto& vert : this->model->bbox)
	{
		min = glm::min(min, vert);
		max = glm::max(max, vert);
	}

	AABBox bbox = AABBox(glm::vec3(min), glm::vec3(max));
	glm::mat4 vpmatrix = projMatrix * cam->GetViewMatrix();
	Frustum frust = Frustum::FromVPMatrix(vpmatrix);

	return Intersect(bbox, frust);
}

CTextureObject::CTextureObject()
{

}

CTextureObject::~CTextureObject()
{
	if (texture)
	{
		texture->ogl_unload();
		delete texture;
		texture = nullptr;
	}
}

void CTextureObject::LoadTM2(Tm2File& tm2)
{
	Tm2Picture& picture = tm2.pictures.at(0);
	this->clutType = (CLT_TYPE)picture.header.clutType;
	this->clutCount = picture.header.clutColors;
	this->clut = std::vector<uint8>(picture.clutData);

	this->imageType = (IMG_TYPE)picture.header.imageType;
	this->imageWidth = picture.header.width;
	this->imageHeight = picture.header.height;
	this->image = std::vector<uint8>(picture.pixelData);

	this->textureWidth = 1 << picture.header.gsTex0b.TW;
	this->textureHeight = 1 << picture.header.gsTex0b.TH;
}

void CTextureObject::CreateTexture()
{
	uint32 width = imageWidth;
	uint32 height = imageHeight;

	uint32 psWidth = textureWidth;
	uint32 psHeight = textureHeight;

	std::vector<uint8> decodedData = std::vector<uint8>(4 * width * height);

	uint8* pixelData = image.data();
	int pixelPtr = 0;
	uint8* clutData = clut.data();
	uint32 rem = (psWidth - width) * 4;

	for (int py = 0; py < height; py++)
	{
		for (int px = 0; px < width; px++)
		{
			// STEP 1: Decode clut index
			uint32 pixelIndex0 = pixelData[pixelPtr];
			uint32 pixelIndex1 = pixelIndex0;

			switch (imageType)
			{
			case IT_RGBA:	// No CLUT.
				break;
			case IT_CLUT4:	// 4 bit CLUT => 2 pixels per byte
				pixelIndex0 &= 0x0F;
				pixelIndex1 = (pixelIndex1 >> 4) & 0x0F;
				break;
			case IT_CLUT8:	// 8 bit CLUT
				if ((pixelIndex0 & 31) >= 8)
				{
					if ((pixelIndex0 & 31) < 16)
					{
						pixelIndex0 += 8;				// +8 - 15 to +16 - 23
					}
					else if ((pixelIndex0 & 31) < 24)
					{
						pixelIndex0 -= 8;				// +16 - 23 to +8 - 15
					}
				}
				break;
			default:
				// TODO: Error
				break;
			}

			// STEP 2: Decode first pixel from data/clut
			uint16 temp16;
			switch (clutType)
			{
			case CT_NONE:
				decodedData.push_back(pixelData[pixelPtr++]);
				decodedData.push_back(pixelData[pixelPtr++]);
				decodedData.push_back(pixelData[pixelPtr++]);
				decodedData.push_back(pixelData[pixelPtr++]);
				break;
			case CT_A1BGR5:
				pixelIndex0 *= 2;
				temp16 = clutData[pixelIndex0 + 0] | (clutData[pixelIndex0 + 1] << 8);
				decodedData.push_back((temp16 & 0x1F) * (1.0 / 31.0 * 255.0));
				decodedData.push_back(((temp16 >> 5) & 0x1F) * (1.0 / 31.0 * 255.0));
				decodedData.push_back(((temp16 >> 10) & 0x1F) * (1.0 / 31.0 * 255.0));
				decodedData.push_back((temp16 & 0x8000) ? (0xFF) : (0));
				break;
			case CT_XBGR8:
				pixelIndex0 *= 3;
				decodedData.push_back(clutData[pixelIndex0 + 0]);
				decodedData.push_back(clutData[pixelIndex0 + 1]);
				decodedData.push_back(clutData[pixelIndex0 + 2]);
				decodedData.push_back(0xFF);
				break;
			case CT_ABGR8:
				pixelIndex0 *= 4;
				decodedData.push_back(clutData[pixelIndex0 + 0]);
				decodedData.push_back(clutData[pixelIndex0 + 1]);
				decodedData.push_back(clutData[pixelIndex0 + 2]);
				decodedData.push_back(clutData[pixelIndex0 + 3]);
				break;
			default:
				// TODO: Error
				break;
			}

			// STEP 3: handle possible 2nd pixel
			if (imageType == IT_CLUT4)
			{
				px++;
				switch (clutType)
				{
					// NOTE: No CT_NONE, as imageType == IT_CLUT4 && clutType == CT_NONE would be invalid
				case CT_A1BGR5:
					pixelIndex0 *= 2;
					temp16 = clutData[pixelIndex0 + 0] | (clutData[pixelIndex0 + 1] << 8);
					decodedData.push_back((temp16 & 0x1F) * (1.0 / 31.0 * 255.0));
					decodedData.push_back(((temp16 >> 5) & 0x1F) * (1.0 / 31.0 * 255.0));
					decodedData.push_back(((temp16 >> 10) & 0x1F) * (1.0 / 31.0 * 255.0));
					decodedData.push_back((temp16 & 0x8000) ? (0xFF) : (0));
					break;
				case CT_XBGR8:
					pixelIndex0 *= 3;
					decodedData.push_back(clutData[pixelIndex0 + 0]);
					decodedData.push_back(clutData[pixelIndex0 + 1]);
					decodedData.push_back(clutData[pixelIndex0 + 2]);
					decodedData.push_back(0xFF);
					break;
				case CT_ABGR8:
					pixelIndex0 *= 4;
					decodedData.push_back(clutData[pixelIndex0 + 0]);
					decodedData.push_back(clutData[pixelIndex0 + 1]);
					decodedData.push_back(clutData[pixelIndex0 + 2]);
					decodedData.push_back(clutData[pixelIndex0 + 3]);
					break;
				default:
					// TODO: Error
					break;
				}
			}
		}

		for (int p = 0; p < rem; p++) decodedData.push_back(0);
	}

	texture = new CTexture(textureWidth, textureHeight, decodedData.data(), PF_RGBA32);
}

CTextureInfo::CTextureInfo(CMap* map, PmpTexEntry& texInfo, CTextureObject* texObj)
{
	parent = map;
	srcTexture = texObj;
	name = std::string(texInfo.name, 12);
	scrollSpeed_x = texInfo.scrollX;
	scrollSpeed_y = texInfo.scrollY;
}

CTextureInfo::~CTextureInfo()
{
	if (srcTexture)
	{
		delete srcTexture;
		srcTexture = nullptr;
	}
}

void CTextureInfo::Update(float deltaTime, double worldTime)
{
	float scrollX = worldTime * scrollSpeed_x * 30.f;
	float scrollY = worldTime * scrollSpeed_y * 30.f;
	currentScroll = glm::vec2(scrollX, scrollY);
}

CModelObject::CModelObject()
{

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

	if (pmo.hasMesh0())
	{
		for (PmoMesh& mesh : pmo.mesh0)
		{
			CModelSection* section = new CModelSection();
			section->LoadSection(mesh);
			sections.push_back(section);
		}
	}

	if (pmo.hasMesh1())
	{
		for (PmoMesh& mesh : pmo.mesh0)
		{
			CModelSection* section = new CModelSection();
			section->LoadSection(mesh);
			transSections.push_back(section);
		}
	}
	
	for (PmoTexture& tex : pmo.textures)
	{
		this->textureNames.push_back(std::string(tex.resourceName, 12));
		//if (loadTextures) LoadTexture(tex);
	}
}

void CModelObject::LinkExtTextures(std::unordered_map<std::string, CTextureInfo*> textureMap)
{
	for (std::string & texName : textureNames)
	{
		CTextureInfo* texObj = textureMap.at(texName);
		textureObjects.push_back(texObj);
	}
}

void CModelObject::UpdateTextureOffsets()
{
	std::vector<glm::vec2> texOffs;
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
	for each (CModelSection * modelSection in sections)
	{
		CMeshSection meshSection;
		meshSection.vertCount = modelSection->vertexCount;
		meshSection.textureIndex = modelSection->textureIndex;
		for each (uint16 kick in modelSection->primCount)
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
				mesh->vertData.push_back(modelSection->vertexData[rp++]);
				mesh->vertData.push_back(modelSection->vertexData[rp++]);
				mesh->vertData.push_back(modelSection->vertexData[rp++]);
			}
			else
			{
				mesh->vertData.push_back(0.0f);
				mesh->vertData.push_back(0.0f);
				mesh->vertData.push_back(0.0f);
			}
		}

		mesh->vertCount += modelSection->vertexCount;
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
	for (auto texEntry : this->textureObjects)
		textureList.push_back(texEntry->srcTexture->texture);
	if (mesh0 != nullptr) mesh0->textures = textureList;
	if (mesh1 != nullptr) mesh1->textures = textureList;
}

void CModelObject::DoDraw(RenderContext& render, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
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

VertexFlags Merge(const VertexFlags& a, const VertexFlags& b)
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

}

void CModelSection::LoadSection(PmoMesh& mesh)
{
	uint32 constDiffuse = 0;

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
		this->flags.hasVColor = 1;
	if (format.texCoord != 0)
		this->flags.hasTex = 1;
	this->flags.hasPosition = 1;

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
			temp16[0] = pVertStart[readPtr++] | (pVertStart[readPtr++] << 8);
			temp16[1] = pVertStart[readPtr++] | (pVertStart[readPtr++] << 8);
			vertexData.push_back((float)temp16[0] / 32767.0f);
			vertexData.push_back((float)temp16[1] / 32767.0f);
			break;
		case 3: // float
			readPtr += ((0x4 - ((readPtr - 0) & 0x3)) & 0x3);
			temp32[0] = pVertStart[readPtr++] | (pVertStart[readPtr++] << 8)
				| (pVertStart[readPtr++] << 16) | (pVertStart[readPtr++] << 24);
			temp32[1] = pVertStart[readPtr++] | (pVertStart[readPtr++] << 8)
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
			temp16[0] = pVertStart[readPtr++] | (pVertStart[readPtr++] << 8);
			vertexData.push_back(((float)((temp16[0] & 0xF800) >> 11) / 31.f) * 255.f);
			vertexData.push_back(((float)((temp16[0] & 0x07E0) >>  5) / 63.f) * 255.f);
			vertexData.push_back(((float)((temp16[0] & 0x001F) >>  0) / 31.f) * 255.f);
			vertexData.push_back(0xFF);
			break;
		case 5: // ABGR-5551
			temp16[0] = pVertStart[readPtr++] | (pVertStart[readPtr++] << 8);
			vertexData.push_back(((float)((temp16[0] & 0xF800) >> 11) / 31.f) * 255.f);
			vertexData.push_back(((float)((temp16[0] & 0x07C0) >>  6) / 31.f) * 255.f);
			vertexData.push_back(((float)((temp16[0] & 0x003E) >>  1) / 31.f) * 255.f);
			vertexData.push_back((temp16[0] & 0x8000) ? (0xFF) : (0));
			break;
		case 6: // ABGR-4444
			temp16[0] = pVertStart[readPtr++] | (pVertStart[readPtr++] << 8);
			vertexData.push_back(((float)((temp16[0] & 0xF000) >> 12) / 15.f) * 255.f);
			vertexData.push_back(((float)((temp16[0] & 0x0F00) >>  8) / 15.f) * 255.f);
			vertexData.push_back(((float)((temp16[0] & 0x00F0) >>  4) / 15.f) * 255.f);
			vertexData.push_back(((float)((temp16[0] & 0x000F) >>  0) / 15.f) * 255.f);
			break;
		case 7: // ABGR-8888
			// TODO: Check color alignment
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
			temp16[0] = pVertStart[readPtr++] | (pVertStart[readPtr++] << 8);
			temp16[1] = pVertStart[readPtr++] | (pVertStart[readPtr++] << 8);
			temp16[2] = pVertStart[readPtr++] | (pVertStart[readPtr++] << 8);
			vertexData.push_back((float)bit_cast<int16, uint16>(temp16[0]) / 32767.0f);
			vertexData.push_back((float)bit_cast<int16, uint16>(temp16[1]) / 32767.0f);
			vertexData.push_back((float)bit_cast<int16, uint16>(temp16[2]) / 32767.0f);
			break;
		case 3:
			readPtr += ((0x4 - ((readPtr - 0) & 0x3)) & 0x3);
			temp32[0] = pVertStart[readPtr++] | (pVertStart[readPtr++] << 8)
				| (pVertStart[readPtr++] << 16) | (pVertStart[readPtr++] << 24);
			temp32[1] = pVertStart[readPtr++] | (pVertStart[readPtr++] << 8)
				| (pVertStart[readPtr++] << 16) | (pVertStart[readPtr++] << 24);
			temp32[2] = pVertStart[readPtr++] | (pVertStart[readPtr++] << 8)
				| (pVertStart[readPtr++] << 16) | (pVertStart[readPtr++] << 24);
			vertexData.push_back(bit_cast<float, uint32>(temp32[0]));
			vertexData.push_back(bit_cast<float, uint32>(temp32[1]));
			vertexData.push_back(bit_cast<float, uint32>(temp32[2]));
		}
	}
}