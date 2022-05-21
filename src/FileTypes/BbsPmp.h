#pragma once
#include <vector>
#include <fstream>
#include <cstdint>
#include "BbsPmo.h"
#include "Tim2.h"

struct PmpHeader
{
    uint32_t magic;
	uint16_t version;
	uint16_t padding0;
	uint32_t padding1;
	uint8_t padding2[3];
	uint8_t flag;
	uint16_t instance_count;
	uint16_t model_count;
	uint32_t padding3;
	uint16_t padding4;
	uint16_t tex_count;
	uint32_t tex_list_offset;
};

struct PmpInstance
{
    float loc[3];
	float rot[3];
	float scale[3];
	uint32_t offset;
	uint32_t padding0;
	uint16_t flags;
	uint16_t model_idx;

    PmoFile data;
};

struct PmpTexEntry
{
	uint32_t offset;
	char name[0xC];
	float scrollX;
	float scrollY;
	uint32_t padding0[2];

    Tm2File data;
};

class PmpFile
{
public:
    PmpHeader header;
    std::vector<PmpInstance> instances;
    std::vector<PmpTexEntry> textures;

	uint16_t instanceCount();
	uint16_t textureCount();

    static PmpFile ReadPmpFile(std::ifstream& file, std::streamoff base = 0);
};