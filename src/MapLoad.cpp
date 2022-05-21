#include "Common.h"
#include <cstdint>
#include <cstring>
#include "imgui.h"
#include "Core\CTexture.h"
#include <unordered_map>
#include "Mesh.h"
#include <algorithm>
#include "MapLoad.h"
#include "AssimpInterface.h"

// FLAGS NOT FOUND:
//	Culling Order,
//	Texture mapping: http://hitmen.c02.at/files/yapspd/psp_doc/index.html#idx11.5.153

#include "BBSTypes.h"

std::string mapname = "";
char * g_fbuf = nullptr;
PMP_HDR * header = nullptr;
OBJ_ENTRY * obj_index = nullptr;
TEX_ENTRY * tex_index = nullptr;

// prep for drawing;
std::vector<std::pair<OBJ_ENTRY *, PMO_HEADER *>> map_objects;
std::vector<std::pair<std::string, TM2_HEADER *>> map_textures;

std::unordered_map<std::string, CTexture *> render_textures;
std::unordered_map<std::string, glm::vec2> texture_scrolls;
std::vector<Mesh *> render_meshes;

CTexture *g_dummyTexture = nullptr;
uint16 g_flagsSeen = 0;
uint8 g_maxTexPathLen = 0;

// tex viewer
std::string tex_target = "";
bool show_tex = false;

unsigned int max_object_id()
{
	return map_objects.size() - 1;
}

void gui_object_view(unsigned int object_id)
{
	if (object_id > max_object_id())
	{
		return;
	}

	OBJ_ENTRY *obj_entry = map_objects[object_id].first;
	PMO_HEADER *obj_pmo = map_objects[object_id].second;
	Mesh *obj_mesh = nullptr;
	if (obj_pmo)	// No point searching for a mesh that cannot exist
	{
		auto pred = [object_id](Mesh *v)->bool { return (v->mesh_idx == object_id); };
		auto find = std::find_if(render_meshes.begin(), render_meshes.end(), pred);
		if (find != render_meshes.end())
			obj_mesh = *find;
	}


	ImGui::Begin("Object Details");

	ImGui::Text("ID: %d", object_id);
	ImGui::Text("Location: {%2.f, %2.f, %2.f}", obj_entry->loc[0], obj_entry->loc[1], obj_entry->loc[2]);
	ImGui::Text("Rotation: {%2.f, %2.f, %2.f}", obj_entry->rot[0], obj_entry->rot[1], obj_entry->rot[2]);
	ImGui::Text("Scale:    {%2.f, %2.f, %2.f}", obj_entry->scale[0], obj_entry->scale[1], obj_entry->scale[2]);
	//ImGui::Text("Unk_28:    0x%08X", obj_entry->unk_28);
	ImGui::Text("Flags:     0x%04X", obj_entry->flags);
	ImGui::Text("Unk_2E:    0x%04X", obj_entry->object_id);

	ImGui::Separator();

	if (!obj_pmo)
	{
		ImGui::Text("No PMO");
	}
	else
	{
		ImGui::Text("Num: %02X Grp: %02X Ver: %02X", obj_pmo->num, obj_pmo->group, obj_pmo->ver);
		ImGui::Text("Texture Count: %d", obj_pmo->textureCount);
		ImGui::Text("Flag: 0x%02X", obj_pmo->unk_flag);
		ImGui::Text("Skeleton Offset: 0x%08X", (intptr_t)(obj_pmo->skeletonOffset));
		ImGui::Text("Mesh 0 Offset: 0x%08X", (intptr_t)(obj_pmo->meshOffset0));
		ImGui::Text("Vertex Count: %d", obj_pmo->vertexCount);
		ImGui::Text("Model Scale: %2.f", obj_pmo->modelScale);
		ImGui::Text("Mesh 1 Offset: 0x%08X", (intptr_t)(obj_pmo->meshOffset1));
		ImGui::Text("Bounding Box: NYI");
	}

	ImGui::Separator();

	if(!obj_mesh)
	{
		ImGui::Text("No Mesh");
	}
	else
	{
		ImGui::Text("Mesh Idx: %d", obj_mesh->mesh_idx);
		ImGui::Text("isSkybox: %s", (obj_mesh->GetIsSkybox() ? "TRUE" : "FALSE"));
		ImGui::Text("Section Count: %d", obj_mesh->GetSectionCount());
		if (ImGui::TreeNode("Section List"))
		{
			obj_mesh->gui_ListSections();
			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void gui_tex_view()
{
	static float tex_scale = 1.0f;

	if (!show_tex) return;

	ImGui::Begin("Texture Viewer", &show_tex);

	if (render_textures.find(tex_target) == render_textures.end())
	{
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "INVALID TEXTURE.");
		ImGui::Text("somehow...");
		ImGui::End();
		return;
	}
	
	CTexture* target = render_textures.at(tex_target);
	target->ogl_loadIfNeeded();
	
	ImGui::Text("Texture Name: %s", tex_target.c_str());
	ImGui::Text("Width: %d", target->getWidth());
	ImGui::Text("Height: %d", target->getHeight());
	ImGui::SliderFloat("Scale", &tex_scale, 0.0f, 4.0f);
	if (target->userPtr)
	{
		if (ImGui::TreeNode("TIM"))
		{
			TM2_HEADER* hdr1 = (TM2_HEADER*)target->userPtr;
			TM2_PICTURE_HEADER* hdr2 = (TM2_PICTURE_HEADER*)(hdr1 + 1);
			ImGui::Text("Image Type: %d", hdr2->imageType);
			ImGui::Text("Clut Type: %d", hdr2->clutType);
			ImGui::TreePop();
		}
	}
	ImGui::Separator();
	ImGui::Image((void *)(intptr_t)(target->getOglId()), ImVec2(target->getWidth() * tex_scale, target->getHeight() * tex_scale), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));

	ImGui::End();
}

void gui_loaded_data()
{
	ImGui::Begin("DATA VIEW");

	if (render_textures.size() > 0)
	{
		if (ImGui::TreeNode("Textures"))
		{
			for (auto& pair : render_textures)
			{
				if (ImGui::TreeNode(pair.first.c_str()))
				{
					ImGui::Text("Size: %d x %d", pair.second->getWidth(), pair.second->getHeight());
					ImGui::Text("Loaded?: %s", (pair.second->isLoaded() ? "TRUE" : "FALSE"));
					if (ImGui::Button("View"))
					{
						pair.second->ogl_loadIfNeeded();
						tex_target = pair.first;
						show_tex = true;
					}
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
	}

	if (render_meshes.size() > 0)
	{
		if (ImGui::TreeNode("Meshes"))
		{
			ImGui::Text("NYI");
			ImGui::Text("%d meshes loaded", render_meshes.size());
			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void gui_map_object(std::pair<OBJ_ENTRY *, PMO_HEADER *> object_entry, int id)
{
	if (ImGui::TreeNode((void*)(intptr_t)id, "Object %d", id))
	{
		ImGui::Text("Location: {%2.f, %2.f, %2.f}", object_entry.first->loc[0], object_entry.first->loc[1], object_entry.first->loc[2]);
		ImGui::Text("Rotation: {%2.f, %2.f, %2.f}", object_entry.first->rot[0], object_entry.first->rot[1], object_entry.first->rot[2]);
		ImGui::Text("Scale:    {%2.f, %2.f, %2.f}", object_entry.first->scale[0], object_entry.first->scale[1], object_entry.first->scale[2]);
		if (ImGui::TreeNode((void *)1, "Flags:    0x%04X", object_entry.first->flags))
		{
			uint16 flags = object_entry.first->flags;
			ImGui::Text("%d", (bool)(flags & (1 << 0))); ImGui::SameLine();
			ImGui::Text("%d", (bool)(flags & (1 << 1))); ImGui::SameLine();
			ImGui::Text("%d", (bool)(flags & (1 << 2))); ImGui::SameLine();
			ImGui::Text("%d", (bool)(flags & (1 << 3))); ImGui::SameLine();
			ImGui::Text("%d", (bool)(flags & (1 << 4))); ImGui::SameLine();
			ImGui::Text("%d", (bool)(flags & (1 << 5))); ImGui::SameLine();
			ImGui::Text("%d", (bool)(flags & (1 << 6))); ImGui::SameLine();
			ImGui::Text("%d", (bool)(flags & (1 << 7)));
			ImGui::Text("%d", (bool)(flags & (1 << 8))); ImGui::SameLine();
			ImGui::Text("%d", (bool)(flags & (1 << 9))); ImGui::SameLine();
			ImGui::Text("%d", (bool)(flags & (1 << 10))); ImGui::SameLine();
			ImGui::Text("%d", (bool)(flags & (1 << 11))); ImGui::SameLine();
			ImGui::Text("%d", (bool)(flags & (1 << 12))); ImGui::SameLine();
			ImGui::Text("%d", (bool)(flags & (1 << 13))); ImGui::SameLine();
			ImGui::Text("%d", (bool)(flags & (1 << 14))); ImGui::SameLine();
			ImGui::Text("%d", (bool)(flags & (1 << 15)));
			ImGui::TreePop();
		}
		ImGui::Text("Unknown B: 0x%04X", object_entry.first->object_id);
		if (ImGui::TreeNode("PMO"))
		{
			if (object_entry.second == nullptr)
			{
				ImGui::Text("Null Object");
			}
			else
			{
				ImGui::Text("Texture Count: %d", object_entry.second->textureCount);
				ImGui::Text("Triangle Count: %d", object_entry.second->triangleCount);
				ImGui::Text("Vertex Count: %d", object_entry.second->vertexCount);
				if (object_entry.second->textureCount)
				{
					if (ImGui::TreeNode("Textures"))
					{
						PMO_TEXTURE_HEADER *base = (PMO_TEXTURE_HEADER *)((char *)object_entry.second + sizeof(PMO_HEADER));
						for (int i = 0; i < object_entry.second->textureCount; i++)
						{
							ImGui::Text(base[i].resourceName);
						}

						ImGui::TreePop();
					}
				}
			}

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}
}

void gui_map_texture(std::pair<std::string, TM2_HEADER *> texture_entry, int id)
{
	if (ImGui::TreeNode(texture_entry.first.c_str()))
	{
		ImGui::Text("Image Count: %d", texture_entry.second->imageCount);

		ImGui::TreePop();
	}
}

void gui_MapData()
{
	ImGui::Begin("BBS MAP");
	if (g_fbuf == nullptr)
	{
		ImGui::Text("NO MAP LOADED");
		ImGui::End();
		return;
	}

	ImGui::Text("All Flags: 0x%04X", g_flagsSeen);
	ImGui::Text("Max Path: 0x%04X", g_maxTexPathLen);
	
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	if (ImGui::TreeNode(mapname.c_str()))
	{
		if (ImGui::TreeNode("Objects", "Objects[%d]", map_objects.size()))
		{
			for (int i = 0; i < map_objects.size(); i++)
			{
				gui_map_object(map_objects[i], i);
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Textures", "Textures[%d]", map_textures.size()))
		{
			for (int i = 0; i < map_textures.size(); i++)
			{
				gui_map_texture(map_textures[i], i);
			}

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	ImGui::End();
}

void RenderBBSMap(RenderContext& context)
{
	for (auto mesh : render_meshes)
	{
		mesh->Draw(context, 0);
	}

	glDepthMask(GL_FALSE);
	for (auto mesh : render_meshes)
	{
		mesh->Draw(context, 1);
	}
	glDepthMask(GL_TRUE);
}

// Almost this entire function is revel8ion code...
void ParseVertex(PMO_MESH_HEADER *meshHeader, uint8 *vertexStart, uint8 *jointIndices, float modelScale, MeshBuilder& builder, uint32 hdr_diffuse)
{
	uint8 *vertexData = vertexStart;
	uint8 *jointWeights = NULL;
	if (meshHeader->dataFlags.skinning)
	{
		//jointWeights = vertexData;

		//rapi->rpgVertBoneIndexUB(jointIndices, meshHeader->dataFlags.jointCnt + 1);
		//rapi->rpgVertBoneWeightUB(jointWeights, meshHeader->dataFlags.jointCnt + 1);

		vertexData += meshHeader->dataFlags.skinning + 1;
	}

	float tempData[4] = { 0 };

	switch (meshHeader->dataFlags.texCoord)
	{
	case 0:
		// No tex coord
		break;
	case 1:
	{
		uint8 *textureCoord = (uint8*)vertexData;
		tempData[0] = (float)textureCoord[0] * (1.0 / 127.0);
		tempData[1] = (float)textureCoord[1] * (1.0 / 127.0);
		//rapi->rpgVertUV2f(tempData, 0);
		builder.TexUV2f(tempData);
		vertexData += 2;
	}
	break;
	case 2:
	{
		vertexData += ((0x2 - ((vertexData - vertexStart) & 0x1)) & 0x1);
		uint16 *textureCoord = (uint16*)vertexData;
		tempData[0] = (float)textureCoord[0] * (1.0 / 32767.0);
		tempData[1] = (float)textureCoord[1] * (1.0 / 32767.0);
		//rapi->rpgVertUV2f(tempData, 0);
		builder.TexUV2f(tempData);
		vertexData += 4;
	}
	break;
	case 3:
	{
		vertexData += ((0x4 - ((vertexData - vertexStart) & 0x3)) & 0x3);
		float *textureCoord = (float*)vertexData;
		tempData[0] = (float)textureCoord[0];
		tempData[1] = (float)textureCoord[1];
		//rapi->rpgVertUV2f(tempData, 0);
		builder.TexUV2f(tempData);
		vertexData += 8;
	}
	break;
	default:
		std::cout << "unknown dataFlags.texCoord value " << meshHeader->dataFlags.texCoord << std::endl;
		break;
	}

	switch (meshHeader->dataFlags.color)
	{
	case 7:
		vertexData += ((0x4 - ((vertexData - vertexStart) & 0x3)) & 0x3);
		{
			float color[4] = { (float)vertexData[0] / 128.0f, (float)vertexData[1] / 128.0f, (float)vertexData[2] / 128.0f, (float)vertexData[3] / 255.0f };
			//rapi->rpgVertColor4f(color);
			builder.Color4f(color);
		}
		vertexData += 4;
		break;
	/*case 2:
		//rapi->rpgVertColor3ub(vertexData);
		builder.Color3ub(vertexData);
		vertexData += 3;
		break;*/
	default:
	{
		if (meshHeader->dataFlags.diffuse)
		{
			uint8 c[4];
			c[0] = (hdr_diffuse >> 0) & 0xFF;
			c[1] = (hdr_diffuse >> 8) & 0xFF;
			c[2] = (hdr_diffuse >> 16) & 0xFF;
			c[3] = (hdr_diffuse >> 24) & 0xFF;
			float color[4] = { (float)c[0] / 128.0f, (float)c[1] / 128.0f, (float)c[2] / 128.0f, (float)c[3] / 255.0f };
			builder.Color4f(color);
		}
		else
		{
			std::cout << "unknown dataFlags.color value " << meshHeader->dataFlags.color << std::endl;
			// set a default white diffuse color to avoid zeroing
			uint8 color[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
			//rapi->rpgVertColor4ub(color);
			builder.Color4ub(color);
		}
	}
	break;
	}

	switch (meshHeader->dataFlags.position)
	{
	case 2:
	{
		vertexData += ((0x2 - ((vertexData - vertexStart) & 0x1)) & 0x1);
		int16 *position = (int16*)vertexData;
		tempData[0] = (float)position[0] * (1.0 / 32767.0) * modelScale;
		tempData[1] = (float)position[1] * (1.0 / 32767.0) * modelScale;
		tempData[2] = (float)position[2] * (1.0 / 32767.0) * modelScale;
		//rapi->rpgVertex3f(tempData);
		builder.Vertex3f(tempData);
		vertexData += 6;
	}
	break;
	case 3:
	{
		vertexData += ((0x4 - ((vertexData - vertexStart) & 0x3)) & 0x3);
		float *position = (float*)vertexData;
		tempData[0] = (float)position[0] * modelScale;
		tempData[1] = (float)position[1] * modelScale;
		tempData[2] = (float)position[2] * modelScale;
		//rapi->rpgVertex3f(tempData);
		builder.Vertex3f(tempData);
		vertexData += 12;
	}
	break;
	default:
		// should hopefully never happen...
		std::cout << "unknown dataFlags.position value " << meshHeader->dataFlags.position << std::endl;
		break;
	}

	builder.EndVert();
}

// more revel8ion code...
void LoadSectionGroup(PMO_HEADER *pmo_header, PMO_MESH_HEADER *mesh_header, int pass, int numBones, MeshBuilder& builder, std::vector<std::string> textures)
{
	int meshIndex = 0;
	while (mesh_header->vertexCount)
	{
		uint8 *vertexStart = (uint8*)(mesh_header + 1);

		uint8 *jointIndices = NULL;
		// if joints are present...
		if (numBones)
		{
			// store address of joint indices
			jointIndices = vertexStart;
			vertexStart += 8;
		}

		uint32 diffuseColor = 0xFFFFFFFF;
		if (mesh_header->dataFlags.diffuse)
		{
			// retrieve mesh diffuse color
			diffuseColor = *((uint32*)vertexStart);
			vertexStart += 4;
		}

		uint32 stripCount = 0;
		uint16 *stripLengths = NULL;
		if (mesh_header->triStripCount)
		{
			// setup multiple tri-strip lists...
			stripCount = mesh_header->triStripCount;
			stripLengths = (uint16*)vertexStart;
			vertexStart += (stripCount << 1);
		}
		else if (mesh_header->dataFlags.primative == 4)
		{
			// simplified access to a single tri-strip...
			stripCount = 1;
			stripLengths = &(mesh_header->vertexCount);
		}

		// determine mesh material index
		//int texIndex = (mesh_header->textureID != 0xFF) ? (mesh_header->textureID + 1) : (0);
		int texIndex = -1;
		CTexture *texture = nullptr;
		glm::vec2 scroll = glm::vec2(0.0f, 0.0f);
		if (mesh_header->textureID == 0xFF)
			texIndex = -1;
		else
			texIndex = mesh_header->textureID;
		if (texIndex >= 0)
		{
			std::string tex_name = textures[texIndex];
			if (render_textures.find(tex_name) == render_textures.end())
				texture = g_dummyTexture;
			else
				texture = render_textures.at(tex_name);

			if (texture_scrolls.find(tex_name) != texture_scrolls.end())
			{
				scroll = texture_scrolls.at(tex_name);
			}
		}
		else
		{
			texture = g_dummyTexture;
		}

		++meshIndex;

		if (stripLengths)
		{
			// process tri-strip data
			uint32 stripNum = 0;
			for (stripNum = 0; stripNum < stripCount; ++stripNum)
			{
				uint32 vertexCount = stripLengths[stripNum];

				if (!vertexCount)
					continue;

				//rapi->rpgBegin(RPGEO_TRIANGLE_STRIP);
				builder.BeginSection(GL_TRIANGLE_STRIP, texture, scroll, pass);
				uint32 vertexNum = 0;
				for (vertexNum = 0; vertexNum < vertexCount; ++vertexNum, vertexStart += mesh_header->vertexSize)
				{
					//Model_MeshVertex(mesh_header, vertexStart, jointIndices, pmo_header->modelScale);
					ParseVertex(mesh_header, vertexStart, jointIndices, pmo_header->modelScale, builder, diffuseColor);
				}
				//rapi->rpgEnd();
				builder.EndSection(mesh_header);
			}
		}
		else
		{
			// process triangle data
			uint32 vertexCount = mesh_header->vertexCount;

			//rapi->rpgBegin(RPGEO_TRIANGLE);
			builder.BeginSection(GL_TRIANGLES, texture, scroll, pass);
			uint32 vertexNum = 0;
			for (vertexNum = 0; vertexNum < vertexCount; ++vertexNum, vertexStart += mesh_header->vertexSize)
			{
				//Model_MeshVertex(mesh_header, vertexStart, jointIndices, pmo_header->modelScale);
				ParseVertex(mesh_header, vertexStart, jointIndices, pmo_header->modelScale, builder, diffuseColor);
			}
			//rapi->rpgEnd();
			builder.EndSection(mesh_header);
		}

		mesh_header = (PMO_MESH_HEADER*)(vertexStart + ((0x4 - (((char *)vertexStart - (char *)pmo_header) & 0x3)) & 0x3));
	}
}

void LoadMapObjects()
{
	if (g_dummyTexture == nullptr)
	{
		uint8 pix[] = { 0xFF, 0xFF, 0xFF, 0xFF };
		g_dummyTexture = new CTexture(1, 1, pix, PF_RGBA32);
	}
	
	for (int obj_id = 0; obj_id < map_objects.size(); obj_id++)
	{
		std::pair<OBJ_ENTRY *, PMO_HEADER *> obj_entry = map_objects[obj_id];
		if (obj_entry.second == nullptr) continue;
		MeshBuilder builder;

		std::vector<std::string> tex_list;

		if (obj_entry.second->textureCount)
		{
			PMO_TEXTURE_HEADER *texture_header = (PMO_TEXTURE_HEADER *)(obj_entry.second + 1);
			int tex_num = 0;
			for (tex_num = 0; tex_num < obj_entry.second->textureCount; ++tex_num, ++texture_header)
			{
				tex_list.push_back(std::string(texture_header->resourceName));
			}
		}

		int numBones = 0;
		if (obj_entry.second->skeletonOffset)
		{
			PMO_SKEL_HEADER *skeletonHeader = (PMO_SKEL_HEADER *)((char *)obj_entry.second + obj_entry.second->skeletonOffset);
			numBones = skeletonHeader->jointCount;
		}
		
		uint32 meshOffsets[2] = { obj_entry.second->meshOffset0, obj_entry.second->meshOffset1 };
		for (uint32 groupNum = 0; groupNum < 2; ++groupNum)
		{
			if (meshOffsets[groupNum])
			{
				//if (meshOffsets[groupNum] >= bufferLen)
				//{ //bad mesh offset
				//	continue;
				//}
				uint8 *groupStart = (uint8 *)((char *)obj_entry.second + meshOffsets[groupNum]);

				//Model_KHBBS_LoadModel(fileBuffer, groupStart, textures, materials, bones, numBones, rapi);
				LoadSectionGroup(obj_entry.second, (PMO_MESH_HEADER *)groupStart, groupNum, numBones, builder, tex_list);
			}
		}

		Mesh *model = builder.Finish();
		glm::vec3 pos = glm::vec3(obj_entry.first->loc[0], obj_entry.first->loc[1], obj_entry.first->loc[2]);
		glm::vec3 rot = glm::vec3(obj_entry.first->rot[0], obj_entry.first->rot[1], obj_entry.first->rot[2]);
		glm::vec3 scale = glm::vec3(obj_entry.first->scale[0], obj_entry.first->scale[1], obj_entry.first->scale[2]);
		model->SetPosRotScale(pos, rot, scale);
		model->mesh_idx = obj_id;
		MeshFlags flags;
		flags.isSkybox = (bool)(obj_entry.first->flags & 1);
		//flags.isDecal = (bool)(obj_entry.first->flags & (1 << 5));
		model->SetFlags(flags);
		g_flagsSeen |= obj_entry.first->flags;
		model->StoreTextureList(tex_list);
		render_meshes.push_back(model);
	}
}

void LoadMapTextures()
{
	for (int tex_id = 0; tex_id < map_textures.size(); tex_id++)
	{
		std::pair<std::string, TM2_HEADER *> tex_entry = map_textures[tex_id];
		if (tex_entry.second->imageCount != 1)
		{
			std::cout << "WARN: texture " << tex_entry.first << " contains more than one image. Only the first will be loaded!" << std::endl;
		}
		if (tex_entry.first.length() > g_maxTexPathLen) g_maxTexPathLen = tex_entry.first.length();
		TM2_PICTURE_HEADER *picture_header = (TM2_PICTURE_HEADER *)(tex_entry.second + 1); // yes, this works.

		// The following is lifted almost directly from revel8n's BBS neosis plugin
		uint32 imageSize = (picture_header->imageSize);

		uint8 *imageData = (uint8*)(picture_header + 1);
		uint8 *clutData = (imageData + imageSize);

		uint32 psWidth = (1 << picture_header->GsTex0b.TW);
		uint32 psHeight = (1 << picture_header->GsTex0b.TH);
		uint32 width = picture_header->width;
		uint32 height = picture_header->height;
		uint32 rem = (psWidth - width) << 2;

		//uint8 *tempImage = (uint8 *)rapi->Noesis_UnpooledAlloc(psWidth * psHeight * 4);
		uint8 *tempImage = (uint8 *)malloc(psWidth * psHeight * 4);
		uint8 *dstPixel = tempImage;

		if (psWidth == 0 || psHeight == 0 || width == 0 || height == 0) continue;

		uint32 _x = 0;
		uint32 _y = 0;
		for (_y = 0; _y < height; ++_y)
		{
			for (_x = 0; _x < width; ++_x, dstPixel += 4, ++imageData)
			{
				uint32 pixelIndex0 = *imageData;
				uint32 pixelIndex1 = pixelIndex0;
				bool needClut = false;
				uint16 pixel = 0;

				switch (picture_header->imageType)
				{
				case IT_A1BGR5:
					pixel = *((uint16*)(imageData));
					dstPixel[0] = (pixel & 0x1F) * (1.0 / 31.0 * 255.0);
					dstPixel[1] = ((pixel >> 5) & 0x1F) * (1.0 / 31.0 * 255.0);
					dstPixel[2] = ((pixel >> 10) & 0x1F) * (1.0 / 31.0 * 255.0);
					dstPixel[3] = (pixel & 0x8000) ? (0xFF) : (0);
					imageData += 1;
					break;
				case IT_XBGR8:
					dstPixel[0] = imageData[0];
					dstPixel[1] = imageData[1];
					dstPixel[2] = imageData[2];
					imageData += 2;
					break;
				case IT_ABGR8:
					dstPixel[0] = imageData[0];
					dstPixel[1] = imageData[1];
					dstPixel[2] = imageData[2];
					dstPixel[3] = imageData[3];
					imageData += 3;
					break;
				case IT_CLUT4:
					needClut = true;
					pixelIndex0 &= 0x0F;
					pixelIndex1 = (pixelIndex1 >> 4) & 0x0F;
					break;
				case IT_CLUT8:
				{
					needClut = true;
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
				}
				break;
				default:
					std::cerr << "UNKNOWN IMAGE TYPE " << picture_header->imageType << std::endl;
					return;
					break;
				}

				if (!needClut) continue;
				
				switch (picture_header->clutType)
				{
				case CT_NONE:
					break;
				case CT_A1BGR5:
					pixelIndex0 <<= 1;
					pixel = *((uint16*)(clutData + pixelIndex0));
					dstPixel[0] = (pixel & 0x1F) * (1.0 / 31.0 * 255.0);
					dstPixel[1] = ((pixel >> 5) & 0x1F) * (1.0 / 31.0 * 255.0);
					dstPixel[2] = ((pixel >> 10) & 0x1F) * (1.0 / 31.0 * 255.0);
					dstPixel[3] = (pixel & 0x8000) ? (0xFF) : (0);
					break;
				case CT_XBGR8:
					pixelIndex0 *= 3;
					dstPixel[0] = clutData[pixelIndex0 + 0];
					dstPixel[1] = clutData[pixelIndex0 + 1];
					dstPixel[2] = clutData[pixelIndex0 + 2];
					dstPixel[3] = 0xFF;
					break;
				case CT_ABGR8:
					pixelIndex0 <<= 2;
					dstPixel[0] = clutData[pixelIndex0 + 0];
					dstPixel[1] = clutData[pixelIndex0 + 1];
					dstPixel[2] = clutData[pixelIndex0 + 2];
					dstPixel[3] = clutData[pixelIndex0 + 3];
					break;
				default:
					std::cerr << "UNKNOWN CLUT TYPE " << picture_header->clutType << std::endl;
					return;
					break;
				}

				if (picture_header->imageType == IT_CLUT4)
				{
					dstPixel += 4;
					++_x;
					switch (picture_header->clutType)
					{
					case CT_A1BGR5:
						pixelIndex1 <<= 1;
						pixel = *((uint16*)(clutData + pixelIndex1));
						dstPixel[0] = (pixel & 0x1F) * (1.0 / 31.0 * 255.0);
						dstPixel[1] = ((pixel >> 5) & 0x1F) * (1.0 / 31.0 * 255.0);
						dstPixel[2] = ((pixel >> 10) & 0x1F) * (1.0 / 31.0 * 255.0);
						dstPixel[3] = (pixel & 0x8000) ? (0xFF) : (0);
						break;
					case CT_XBGR8:
						pixelIndex1 *= 3;
						dstPixel[0] = clutData[pixelIndex1 + 0];
						dstPixel[1] = clutData[pixelIndex1 + 1];
						dstPixel[2] = clutData[pixelIndex1 + 2];
						dstPixel[3] = 0xFF;
						break;
					case CT_ABGR8:
						pixelIndex1 <<= 2;
						dstPixel[0] = clutData[pixelIndex1 + 0];
						dstPixel[1] = clutData[pixelIndex1 + 1];
						dstPixel[2] = clutData[pixelIndex1 + 2];
						dstPixel[3] = clutData[pixelIndex1 + 3];
						break;
					default:
						std::cerr << "UNKNOWN CLUT TYPE " << picture_header->clutType << std::endl;
						return;
						break;
					}
				}
			}
			dstPixel += rem;
		}
		// end of revel8n code.

		CTexture *actualTexture = new CTexture(psWidth, psHeight, tempImage, PF_RGBA32, tex_entry.second);
		render_textures.emplace(tex_entry.first, actualTexture);
		free(tempImage);
	}
}

void ParseLoadedMap()
{
	if (g_fbuf == nullptr) return;
	
	header = (PMP_HDR *)g_fbuf;

	if (memcmp(header->magic, "PMP\0", 4) != 0)
	{
		std::cerr << "Not a valid map file!" << std::endl;
		return;
	}

	obj_index = (OBJ_ENTRY *)(g_fbuf + sizeof(PMP_HDR));
	tex_index = (TEX_ENTRY *)(g_fbuf + header->tex_index_offset);

	int obj_count = 0;
	for (int obj_id = 0; obj_id < header->obj_count; obj_id++)
	{
		OBJ_ENTRY object = obj_index[obj_id];
		PMO_HEADER * pmo = (PMO_HEADER *)(g_fbuf + object.offset);
		if (memcmp(pmo->fileTag, "PMO\0", 4) != 0)
		{
			if (object.offset != 0)
			{
				std::cerr << "Object id " << obj_id << "@" << (uint32)pmo << " is INVALID" << std::endl;
			}
			else
			{
				std::cerr << "Null object @ id " << obj_id << std::endl;
			}
			pmo = nullptr;
		}
		
		obj_count++;
		map_objects.push_back(std::make_pair(&(obj_index[obj_id]), pmo));
		
	}
	std::cout << "Parse found " << obj_count << " objects" << std::endl;

	int tex_count = 0;
	for (int tex_id = 0; tex_id < header->tex_count; tex_id++)
	{
		TEX_ENTRY texture = tex_index[tex_id];
		TM2_HEADER * tim2 = (TM2_HEADER *)(g_fbuf + texture.offset);
		if (memcmp(tim2, "TIM2", 4) != 0)
		{
			std::cerr << "Texture id " << tex_id << "@" << (uint32)tim2 << " is INVALID" << std::endl;
		}
		else
		{
			tex_count++;
			map_textures.push_back(std::make_pair(texture.name, tim2));
		}
		if (texture.scrollX != 0.0f || texture.scrollY != 0.0f)
		{
			texture_scrolls.emplace(texture.name, glm::vec2(texture.scrollX, texture.scrollY));
		}
	}
	std::cout << "Parse found " << tex_count << " textures" << std::endl;

}

void LoadBBSMap(std::string filepath)
{
	FILE *fp;
	char * filebuffer;

	fp = fopen(filepath.c_str(), "rb");
	if (fp == nullptr)
	{
		std::cerr << "Error opening map file" << std::endl;
		return;
	}

	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	rewind(fp);

	filebuffer = (char *)malloc(sizeof(char) * fsize);
	if (filebuffer == nullptr)
	{
		std::cerr << "Error allocating map memory" << std::endl;
		fclose(fp);
		return;
	}

	size_t result = fread(filebuffer, 1, fsize, fp);
	if (result != fsize)
	{
		std::cerr << "Read error" << std::endl;
		free(filebuffer);
		fclose(fp);
		return;
	}

	size_t last_slash = filepath.find_last_of("/\\");
	if (last_slash == std::string::npos) mapname = filepath;
	else mapname = filepath.substr(last_slash + 1);
	g_fbuf = filebuffer;

}

void UnloadBBSMap()
{
	for (auto& mesh : render_meshes)
	{
		delete mesh;
		mesh = nullptr;
	}
	render_meshes.clear();

	for (auto& tex : render_textures)
	{
		if (tex.second && tex.second->isLoaded()) tex.second->ogl_unload();
		delete (tex.second);
		tex.second = nullptr;
	}
	render_textures.clear();

	map_textures.clear();
	map_objects.clear();
	tex_index = nullptr;
	obj_index = nullptr;
	header = nullptr;
	free(g_fbuf);
	g_fbuf = nullptr;
}

void ExportMap(std::string folder)
{
	AssimpExporter exp;
	exp.BeginExport();
	for (auto& tex : render_textures)
	{
		exp.AddTexture(tex.first, tex.second);
	}
	for (auto& msh : render_meshes)
	{
		exp.AddMesh(msh);
	}
	exp.EndExport(folder, mapname);
}