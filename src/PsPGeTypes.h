#pragma once
#include <cstdint>

enum GE_PrimativeType
{
	PRIM_POINTS = 0,
	PRIM_LINES,
	PRIM_LINE_STRIP,
	PRIM_TRIANGLES,
	PRIM_TRIANGLE_STRIP,
	PRIM_TRIANGLE_FAN,
	PRIM_SPRITES
};

struct GeVTYPE
{
	uint32_t textureFormat : 2;
	uint32_t colorFormat : 3;
	uint32_t normalFormat : 2;
	uint32_t positionFormat : 2;
	uint32_t weightFormat : 2;
	uint32_t indexFormat : 2;

};