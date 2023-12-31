#pragma once
#include <vector>
#include <fstream>
#include <cstdint>
#include "CmnTypes.h"

struct BcdHeader
{
	uint32_t magic;
	uint32_t version;
	int32_t dataCount;
	uint32_t dataOffset;
};

struct ATTR_BIT
{
	uint32_t uiKind : 4;
	uint32_t uiMaterial : 5;
	uint32_t uiHitPlayer : 1;
	uint32_t uiHitEnemy : 1;
	uint32_t uiHitFlyEnemy : 1;
	uint32_t uiHitAttack : 1;
	uint32_t uiHitCamera : 1;
	uint32_t uiDangle : 1;
	uint32_t uiLadder : 1;
	uint32_t uiBarrier : 1;
	uint32_t uiIK : 1;
	uint32_t uiHitPrize : 1;
	uint32_t uiSwim : 1;
	uint32_t uiNoPut : 1;
	uint32_t uiHitGimmick : 1;
	uint32_t dummy : 10;
};

struct BcdPlane
{
	float vPlane[4];
	uint16_t vertIdx[4];
	uint16_t colorIdx[4];
	int32_t info;
	ATTR_BIT attrib;
	uint32_t pad[2];

	bool IsQuad();
};

struct BcdData
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
	float cellSize;
	uint16_t flags;
	int16_t divX, divY, divZ;
	int16_t vertCount;
	int16_t planeCount;
	uint32_t vertexOffset;
	uint32_t planeOffset;
	uint32_t colorOffset;
	uint32_t bitArrayXOffset;
	uint32_t bitArrayYOffset;
	uint32_t bitArrayZOffset;

	std::vector<FVECTOR4> verts;
	std::vector<BcdPlane> planes;
	std::vector<ICOLOR> colors;
};

class BcdFile
{
public:
	BcdHeader header;
	std::vector<BcdData> data;

	static BcdFile ReadBcdFile(std::ifstream& file, std::streamoff base);
};