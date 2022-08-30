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
#include "Core\DebugDraw.h"

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

	PmpFile pmpFile = PmpFile::ReadPmpFile(fileStream);
	std::cout << "[MAP] Loaded PMP: " << filePath << std::endl;

	this->instances.reserve(pmpFile.header.instance_count);
	this->objects.resize(pmpFile.header.model_count);	// Note: resize not reserve because we're going to index into it directly and treat the vector like an array.
	this->textures.reserve(pmpFile.header.tex_count);

	// TODO: this->objects would be better as some kind of sparse array
	// Create instances, loading the actual models as we go
	for (PmpInstance& inst : pmpFile.instances)
	{
		if (inst.offset != 0)
		{
			if (inst.model_idx > objects.size() - 1)
			{
				std::cout << "[MAP] Objects vector resized from " << objects.size() << " to " << inst.model_idx << std::endl;
				objects.resize(inst.model_idx + 1);
			}
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
		texObj->CreateTexture();
		CTextureInfo* mapTexture = new CTextureInfo(tex, texObj);
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
		if (model != nullptr) 
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
			//DebugDraw::DebugCube(*context.render,
			//	instance->boundingBox.center(), glm::vec3(0.0f), instance->boundingBox.halfExtents() * 2.0f * instance->scale, glm::vec3(1.0, 0.0, 0.0));

			//if (instance->BBox(context.render->render.current_camera, context.render->render.projectionMatrix))
			//{
			if (instance->model != nullptr)
			{
				if (instance->model->mesh0 != nullptr)
					context.render->render.staticDrawList.push_back(instance);
				if (instance->model->mesh1 != nullptr)
					context.render->render.dynamicDrawList.push_back(instance);
			}
			//}
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

	instances.clear();
	objects.clear();
	textures.clear();
}

CMapInstance::CMapInstance(CMap* map, PmpInstance& inst)
	: boundingBox(glm::vec3(0.0f), glm::vec3(0.0f))
{
	this->parent = map;
	this->position = glm::vec3(inst.loc[0], inst.loc[1], inst.loc[2]);
	this->rotation = glm::vec3(inst.rot[0], inst.rot[1], inst.rot[2]);
	this->scale = glm::vec3(inst.scale[0], inst.scale[1], inst.scale[2]);
	this->objectID = inst.model_idx;
	this->flags = inst.flags;

	this->model = this->parent->objects[this->objectID];
	if (this->model) this->boundingBox = AABBox::Translate(this->model->boundingBox, this->position);
}

void CMapInstance::DoDraw(RenderContext& render)
{
	if (model)
		model->DoDraw(render, position, rotation, scale);
}

float CMapInstance::CalcZ(RenderContext& context)
{
	return glm::dot(this->position - context.render.current_camera->Position, context.render.current_camera->Front);
}

void CMapInstance::Update(float deltaTime, double worldTime)
{
	// Don't think this needs to do anything?
}

bool CMapInstance::BBox(CCamera* cam, glm::mat4 projMatrix)
{
	// TODO: Disabled for now
	return true;
	
	glm::mat4 vpmatrix = projMatrix * cam->GetViewMatrix();
	Frustum frust = Frustum::FromVPMatrix(vpmatrix);

	return Intersect(this->model->boundingBox, frust);
}
