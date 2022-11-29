#pragma once
#include <vector>
#include <fstream>
#include <cstdint>

struct BcdHeader
{
	uint32_t magic;
	uint32_t version;
	int32_t dataCount;
	uint32_t dataOffset;
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

	std::vector<FVECTOR3> verts;
	std::vector<BcdPlane> planes;
	std::vector<ICOLOR> colors;
};

struct BcdPlane
{
	float vPlane[4];
	uint16_t vertIdx[4];
	uint16_t colorIdx[4];
	int32_t info;
	uint32_t attrib;
	uint32_t pad[2];
};

class BcdFile
{
public:
	BcdHeader header;
	std::vector<BcdData> data;

	static BcdFile ReadBcdFile(std::ifstream& file, std::streamoff base);
};