#pragma once
#include "..\Common.h"
#include "..\Core\CoreRender.h"
#include "Core\World.h"
#include "CTextureObject.h"
#include "CTextureInfo.h"
#include "CModelObject.h"

namespace BBS
{

	// TODO: In memory version of a pmo
	//		 serves as a intermediate object to load pmo data into
	//		 either from pmps, pmos, or fbxs that we're importing.
	//		 basically amounts to a pair of vectors of sections
	//		 that store vertex data + rendering flags + channel info
	//		 (this vert has position, color etc.)
	//class CModelObject;

	// TODO: In memory version of a tim2
	//		 Should probably store info in as close to original format
	//		 as possible. Unlike vertex data (which can just be dumped
	//		 straight to floats) there's no obvious common denominator format
	//		 (except RGBA8888 but then you lose palette info)
	//class CTextureObject;

	class CMapInstance;
	//class CTextureInfo;

	class CMap
	{
	public:

		std::vector<CModelObject*> objects;
		std::unordered_map<std::string, CTextureInfo*> textures;
		std::vector<CMapInstance*> instances;

		void Update(WorldContext& context);
		//void Render(/*RenderContext& context*/);

		void LoadMapFile(std::string filePath);
		void Clear();
	};

	// An instance of a pmo in the scene. The properties of this map directly
	// onto the pmp's equivilant structure.
	class CMapInstance : public CRenderObject
	{
		//CRenderObject
		virtual void DoDraw(RenderContext& context);
		virtual float CalcZ(RenderContext& context);

	protected:
		CMap* parent;
	public:
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
		unsigned int objectID;
		uint16_t flags;

		//CMapObject* renderObject;
		CModelObject* model;

		CMapInstance(CMap* map, PmpInstance& inst);
		void Update(float deltaTime, double worldTime);

		bool BBox(CCamera* cam, glm::mat4 projMatrix);
	};

}