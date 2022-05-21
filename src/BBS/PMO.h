#pragma once
#include "..\BTypes.h"
#include "Types.h"

constexpr uint16 bbs_pmo_ver = 0;
constexpr uint32 PMO_MAGIC_I = ('P' << 24) | ('M' << 16) | ('O' << 8) | ('\0' << 0);
constexpr char PMO_MAGIC_C[4] = { 'P', 'M', 'O', '\0' };

#pragma pack(push, 1)

struct PMO_VERTEX_FLAGS
{
	uint32 texCoord : 2;		// Texture Coordinate Format
								//		0 NONE
								//		1 8-bit fixed
								//		2 16-bit fixed
								//		3 32-bit float

	uint32 color : 3;			// Color Format
								//		0 NONE
								//		4 16-bit BGR-5650
								//		5 16-bit ABGR-5551
								//		6 16-bit ABGR-4444
								//		7 32-bit ABGR-8888

	uint32 normal : 2;			// Normal Format	UNUSED IN BBS

	uint32 position : 2;		// Position Format
								//		0 NONE
								//		1 8-bit fixed
								//		2 16-bit fixed
								//		3 32-bit float

	uint32 weights : 2;			// Weight Format
								//		0 NONE
								//		1 8-bit fixed
								//		2 16-bit fixed
								//		3 32-bit float

	uint32 index : 2;			// Index Format		UNUSED IN BBS

	uint32 unused_b13 : 1;

	uint32 skinning : 3;		// Number of skinning weights
								//		1 to 8. (ie: actual value is this+1)

	uint32 unused_b17 : 1;

	uint32 morphing : 3;		// Number of morphing weights	UNUSED IN BBS

	uint32 unused_b21_22 : 2;

	uint32 skipTransform : 1;	// Skip Transform Pipeline	UNUSED IN BBS?

	uint32 diffuse : 1;			// Diffuse color follows header

	uint32 unk : 3;				// Possibly unused

	uint32 primative : 4;		// Primative Type
								//		0 Points
								//		1 Lines
								//		2 Line Strip
								//		3 Triangles
								//		4 Triangle Strip
								//		5 Triangle Fan
								//		6 Sprites (quads)

};

struct PMO_HDR
{
	uint32 magic;
	uint8 num;
	uint8 group;
	uint8 version;
	uint8 reserved0;
	uint8 tex_max;
	uint8 reserved2;
	uint16 flag;
	uint32 p_skeleton;

	uint32 p_geom;
	uint16 poly_count;
	uint16 vert_count;
	float scale;
	uint32 p_transGeom;
	FVECTOR4 bounding_box[8];
};

struct PMO_TEXINFO
{
	uint32 p_texture;
	char name[12];
	float scrollX;
	float scrollY;
	uint32 reserved0[2];
};

struct PMO_GEOM_HDR
{
	uint16 vert_count;
	uint8 tex_idx;
	uint8 vert_stride;
	PMO_VERTEX_FLAGS vert_format;
	uint8 group;
	uint8 primative_count;
	uint16 attributes;
};

#pragma pack(pop)