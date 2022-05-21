#pragma once
#include "..\BTypes.h"

#pragma pack(push, 1)

struct FVECTOR2
{
	float x, y;
};

struct FVECTOR3
{
	float x, y, z;
};

struct FVECTOR4
{
	float x, y, z, w;
};

struct FMATRIX22
{
	FVECTOR2 v1, v2;
};

struct FMATRIX33
{
	FVECTOR3 v1, v2, v3;
};

struct FMATRIX44
{
	FVECTOR4 v1, v2, v3, v4;
};

union ICOLOR {
    uint32 c;
    struct {
        uint8 r;
        uint8 g;
        uint8 b;
        uint8 a;
    } v;
};

#pragma pack(pop)