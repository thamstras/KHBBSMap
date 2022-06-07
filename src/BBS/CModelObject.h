#pragma once
#include "Common.h"
#include "FileTypes/BbsPmo.h"
#include "Core/CoreRender.h"
#include "Core/CMesh.h"
#include <unordered_map>
#include "CTextureInfo.h"
#include "Utils\Math.h"

namespace BBS
{

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
		AABBox boundingBox;

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