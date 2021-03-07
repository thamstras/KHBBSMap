#pragma once
#include "BTypes.h"

#pragma pack(push, 1)

enum GsPSM
{
	// Pixel Storage Format (0 = 32bit RGBA)

	GS_PSMCT32 = 0,
	GS_PSMCT24 = 1,
	GS_PSMCT16 = 2,
	GS_PSMCT16S = 10,
	GS_PSMT8 = 19,
	GS_PSMT4 = 20,
	GS_PSMT8H = 27,
	GS_PSMT4HL = 36,
	GS_PSMT4HH = 44,
	GS_PSMZ32 = 48,
	GS_PSMZ24 = 49,
	GS_PSMZ16 = 50,
	GS_PSMZ16S = 58,
};

enum IMG_TYPE
{
	IT_A1BGR5 = 1,
	IT_XBGR8 = 2,
	IT_ABGR8 = 3,
	IT_CLUT4 = 4,
	IT_CLUT8 = 5,
};

enum CLT_TYPE
{
	CT_NONE = 0,
	CT_A1BGR5 = 1,
	CT_XBGR8 = 2,
	CT_ABGR8 = 3,
};

typedef struct
{
	uint64 TBP0 : 14; // Texture Buffer Base Pointer (Address/256)
	uint64 TBW : 6; // Texture Buffer Width (Texels/64)
	uint64 PSM : 6; // Pixel Storage Format (0 = 32bit RGBA)
	uint64 TW : 4; // width = 2^TW
	uint64 TH : 4; // height = 2^TH
	uint64 TCC : 1; // 0 = RGB, 1 = RGBA
	uint64 TFX : 2; // TFX  - Texture Function (0=modulate, 1=decal, 2=hilight, 3=hilight2)
	uint64 CBP : 14; // CLUT Buffer Base Pointer (Address/256)
	uint64 CPSM : 4; // CLUT Storage Format
	uint64 CSM : 1; // CLUT Storage Mode
	uint64 CSA : 5; // CLUT Offset
	uint64 CLD : 3; // CLUT Load Control
} GsTex0;

// TM2 File Structure

struct TM2_HEADER
{
	char   fileID[4]; // usually 'TIM2' or 'CLT2'
	uint8  version;   // format version
	uint8  id;        // format id
	uint16 imageCount;
	uint32 padding[2];
};

struct TM2_PICTURE_HEADER
{
	// 0x00
	uint32 totalSize;
	uint32 clutSize;
	uint32 imageSize;
	uint16 headerSize;
	uint16 clutColors;
	// 0x10
	uint8  format;
	uint8  mipMapCount;
	uint8  clutType;
	uint8  imageType;
	uint16 width;
	uint16 height;

	GsTex0 GsTex0b; //[8];
	// 0x20
	GsTex0 GsTex1b; //[8];
	uint32 GsRegs;
	uint32 GsTexClut;
};

struct PMO_TEXTURE_HEADER
{
	// 0x00
	uint32 dataOffset;
	char   resourceName[0x0C];
	// 0x10
	uint32 unknown0x10[4];
};

struct PMO_JOINT
{
	uint16 index;
	uint16 padding0;

	uint16 parent;
	uint16 padding1;

	uint16 unknown0x08; // skinning index?
	uint16 padding2;

	uint32 unknown0x0C;

	// 0x10
	char   name[0x10];
	// 0x20
	float  transform[16];
	float  transformInverse[16];
};

struct PMO_SKEL_HEADER
{
	char   dataTag[4]; // 'BON'
	uint32 unknown0x04;
	uint32 jointCount;
	uint16 unknown0x0C; // number of skinned joints?
	uint16 unknown0x0E; // skinning start index
};

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

struct PMO_MESH_HEADER
{
	// 0x00
	uint16 vertexCount;
	uint8  textureID;
	uint8  vertexSize;

	PMO_VERTEX_FLAGS dataFlags;

	uint8  unknown0x08;
	uint8  triStripCount;
	uint8  unknown0x0A[2];
};

constexpr uint32 PMO_MAGIC_I = ('P' << 24) | ('M' << 16) | ('O' << 8) | ('\0' << 0);
constexpr char PMO_MAGIC_C[4] = { 'P', 'M', 'O', '\0' };

struct PMO_HEADER
{
	// 0x00
	char   fileTag[4]; // 'PMO'
	uint8  unknown0x04[4];
	uint16 textureCount; //
	uint16 unknown0x0A;
	uint32 skeletonOffset;
	// 0x10
	uint32 meshOffset0;
	uint16 triangleCount;
	uint16 vertexCount;
	float  modelScale;
	uint32 meshOffset1;
	// 0x20
	float  boundingBox[8][4];
};

struct PMP_HDR
{
	char magic[4];
	uint32 unk_04;
	uint32 unk_08;
	uint32 unk_0C;
	uint16 obj_count;
	uint16 unk_12;
	uint32 unk_14;
	uint16 unk_18;
	uint16 tex_count;
	uint32 tex_index_offset;
};

struct ObjFlags
{
	uint16 isSkybox : 1;	// render this object as part of the skybox
	uint16 unknown0 : 1;	// not seen yet
	uint16 unknown1 : 1;	// set on floors. Recieve shadow?
	uint16 unknown2 : 1;	// something alpha related?										<|
	uint16 unknown3 : 1;	// unkown, used on flower beds, so probably also alpha related.	<-- BLEND MODE?
	uint16 unknown4 : 1;	// something else alpha related.								<|
	uint16 unknown5 : 1;
	uint16 unknown6 : 1;

	uint16 unknown7 : 1;
	uint16 unknown8 : 1;
	uint16 unknown9 : 1;
	uint16 unknownA : 1;
	uint16 unknownB : 1;
	uint16 unknownC : 1;
	uint16 unknownD : 1;
	uint16 unknownE : 1;
};

struct OBJ_ENTRY
{
	float loc[3];
	float rot[3];
	float scale[3];
	uint32 offset;
	uint32 unk_28;	// Seems to always be 0
	//uint32 unk_2C;
	uint16 flags;
	uint16 unk_2E; 	// Probably an object id number
};

struct TEX_ENTRY
{
	uint32 offset;
	char name[28];
};

#pragma pack(pop)