#pragma once
#include "..\Common.h"
#include <unordered_map>
//#include "..\Texture.h"
#include "..\Core\CoreRender.h"
#include "..\Core\CMesh.h"
//#include "..\Mesh.h"
#include "Core\CTexture.h"
#include "Core\World.h"
#include "PMO.h"
#include "FileTypes/Tim2.h"
#include "FileTypes/BbsPmo.h"
#include "FileTypes/BbsPmp.h"

namespace BBS
{

	// TODO: In memory version of a pmo
	//		 serves as a intermediate object to load pmo data into
	//		 either from pmps, pmos, or fbxs that we're importing.
	//		 basically amounts to a pair of vectors of sections
	//		 that store vertex data + rendering flags + channel info
	//		 (this vert has position, color etc.)
	class CModelObject;

	// TODO: In memory version of a tim2
	//		 Should probably store info in as close to original format
	//		 as possible. Unlike vertex data (which can just be dumped
	//		 straight to floats) there's no obvious common denominator format
	//		 (except RGBA8888 but then you lose palette info)
	class CTextureObject;

	class CMapInstance;
	class CTextureInfo;

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

	class CTextureObject
	{
	public:
		CTextureObject();
		~CTextureObject();

		void LoadTM2(Tm2File& tm2);
		void CreateTexture();

		CTexture* texture;

		CLT_TYPE clutType;
		uint16 clutCount;
		std::vector<uint8> clut;

		IMG_TYPE imageType;
		uint16 imageWidth;
		uint16 imageHeight;
		std::vector<uint8> image;

		uint32 textureWidth;
		uint32 textureHeight;
	};
	
	class CTextureInfo
	{
	protected:
		CMap* parent;
	public:
		CTextureObject* srcTexture;
		std::string name;
		float scrollSpeed_x, scrollSpeed_y;
		glm::vec2 currentScroll;

		CTextureInfo(CMap* map, PmpTexEntry& texInfo, CTextureObject* texObj);
		~CTextureInfo();
		void Update(float deltaTime, double worldTime);
	};
	
	class CModelSection;

	class CModelObject
	{
	public:
		CModelObject();
		~CModelObject();
		void LoadPmo(PmoFile& pmo, bool loadTextures);
		// TODO: void LoadTexture(PMO_TEXINFO* pTexInfo);
		void LinkExtTextures(std::unordered_map<std::string, CTextureInfo*> textureMap);
		void UpdateTextureOffsets();

		void BuildMesh();
		void DoDraw(RenderContext& context, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale);

		float scale;
		glm::vec4 bbox[8];

		std::vector<CModelSection*> sections;
		std::vector<CModelSection*> transSections;
		std::vector<std::string> textureNames;
		bool ownsTextures;
		std::vector<CTextureInfo*> textureObjects;

		CMesh* mesh0;
		CMesh* mesh1;

	private:
		CMesh* BuildMesh(std::vector<CModelSection*>& sections);
	};

	struct VertexFlags
	{
		uint8 hasWeights : 1;
		uint8 hasTex : 1;
		uint8 hasVColor : 1;
		uint8 hasPosition : 1;
		uint8 hasCColor : 1;
	};

	inline bool operator==(const VertexFlags& left, const VertexFlags& right)
	{
		return (left.hasWeights == right.hasWeights)
			&& (left.hasTex == right.hasTex)
			&& (left.hasVColor == right.hasVColor)
			&& (left.hasPosition == right.hasPosition)
			&& (left.hasCColor == right.hasCColor);
	}

	inline bool operator!=(const VertexFlags& left, const VertexFlags& right)
	{
		return !(left == right);
	}

	VertexFlags Merge(const VertexFlags& a, const VertexFlags& b);

	class CModelSection
	{
	public:
		int textureIndex;
		int vertexCount;
		GLenum primativeType;
		std::vector<uint16> primCount;
		VertexFlags flags;
		std::vector<float> vertexData;
		uint16 attributes;
		uint32 globalColor;

		CModelSection();
		void LoadSection(PmoMesh& mesh);
	};

}