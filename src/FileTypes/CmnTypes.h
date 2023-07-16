#pragma once
#include <cstdint>
#include <fstream>

struct FVECTOR3
{
	float x, y, z;
};

FVECTOR3 ParseFVector3(std::ifstream& file);

struct FVECTOR4
{
    float x, y, z, w;
};

FVECTOR4 ParseFVector4(std::ifstream& file);

struct ICOLOR
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

ICOLOR ParseIColor(std::ifstream& file);