#pragma once
#include <vector>
#include <fstream>
#include <cstdint>

struct PmoHeader
{
    uint32_t magic;
    uint8_t unk_04[4];      // TODO: This is num, group, version, padding
    uint16_t textureCount;
    uint16_t padding1;
    uint32_t skeletonOffset;
    uint32_t mesh0Offset;
    uint16_t triangleCount;
    uint16_t vertexCount;
    float scale;
    uint32_t mesh1Offset;
    float boundingBox[8*4];
};

struct PmoSkelHeader
{
    uint32_t magic;
    uint32_t padding0;
    uint16_t jointCount;
    uint16_t padding1;
    uint16_t asn_bone_max;          // TODO: better name
    uint16_t nStdBone;              // TODO: better name
};

struct PmoJoint
{
    uint16_t index;
    uint16_t padding0;
    uint16_t parent;
    uint16_t padding1;
    uint16_t asnnum;                // TODO: better name
    uint16_t padding2;
    uint32_t padding3;
    char name[0x10];
    float transform[4][4];          // std
    float transformInverse[4][4];   // wl
};

struct PmoSkeleton
{
    PmoSkelHeader header;
    std::vector<PmoJoint> joints;
};

struct PmoVertexFormatFlags
{
    uint32_t texCoord : 2;		// Texture Coordinate Format
                                //		0 NONE
                                //		1 8-bit fixed
                                //		2 16-bit fixed
                                //		3 32-bit float

    uint32_t color : 3;			// Color Format
                                //		0 NONE
                                //		4 16-bit BGR-5650
                                //		5 16-bit ABGR-5551
                                //		6 16-bit ABGR-4444
                                //		7 32-bit ABGR-8888

    uint32_t normal : 2;		// Normal Format	UNUSED IN BBS

    uint32_t position : 2;		// Position Format
                                //		0 NONE
                                //		1 8-bit fixed
                                //		2 16-bit fixed
                                //		3 32-bit float

    uint32_t weights : 2;		// Weight Format
                                //		0 NONE
                                //		1 8-bit fixed
                                //		2 16-bit fixed
                                //		3 32-bit float

    uint32_t index : 2;			// Index Format		UNUSED IN BBS

    uint32_t unused_b13 : 1;

    uint32_t skinning : 3;		// Number of skinning weights
                                //		1 to 8. (ie: actual value is this+1)

    uint32_t unused_b17 : 1;

    uint32_t morphing : 3;		// Number of morphing weights	UNUSED IN BBS

    uint32_t unused_b21_22 : 2;

    uint32_t skipTransform : 1;	// Skip Transform Pipeline	UNUSED IN BBS?

    uint32_t diffuse : 1;		// Diffuse color follows header

    uint32_t unk : 3;			// 

    uint32_t primative : 4;		// Primative Type
                                //		0 Points
                                //		1 Lines
                                //		2 Line Strip
                                //		3 Triangles
                                //		4 Triangle Strip
                                //		5 Triangle Fan
                                //		6 Sprites (quads)

};

enum PMO_MESH_ATTR
{
    ATTR_BLEND_NONE = 0x0,
    ATTR_NOMATERIAL = 0x1,
    ATTR_GLARE = 0x2,
    ATTR_BACK = 0x4,
    ATTR_DIVIDE = 0x8,
    ATTR_TEXALPHA = 0x10,
    ATTR_BLEND_SEMITRANS = 0x20,
    ATTR_BLEND_ADD = 0x40,
    ATTR_BLEND_SUB = 0x60,
    ATTR_BLEND_MASK = 0xe0,
    ATTR_8 = 0x100,
    ATTR_9 = 0x200,
    ATTR_DROPSHADOW = 0x400,
    ATTR_ENVMAP = 0x800,
    ATTR_12 = 0x1000,
    ATTR_13 = 0x2000,
    ATTR_14 = 0x4000,
    ATTR_15 = 0x8000,

    FLAG_COLOR = 0x1000000,
    FLAG_NOWEIGHT = 0x2000000,
    FLAG_SHIFT = 0x18,
    PRIM_SHIFT = 0x1c,
};

struct PmoMeshHeader
{
    uint16_t vertexCount;
    uint8_t textureIndex;
    uint8_t vertexSize;
    PmoVertexFormatFlags vertexFormat;
    uint8_t unk_08;                 // group
    uint8_t triStripCount;
    uint16_t attributes;
    uint8_t jointIndices[8];
    uint32_t diffuseColor;
    std::vector<uint16_t> triStripLengths;
};

struct PmoMesh
{
    PmoMeshHeader header;
    std::vector<uint8_t> vertexData;
};

struct PmoTexture
{
    uint32_t dataOffset;
    char resourceName[0xC];
    uint32_t unk_10[4];
};

class PmoFile
{
public:
    PmoHeader header;
    std::vector<PmoTexture> textures;
    std::vector<PmoMesh> mesh0;
    std::vector<PmoMesh> mesh1;
    PmoSkeleton skeleton;

    static PmoFile ReadPmoFile(std::ifstream& file, std::streamoff base = 0);

    bool hasTextures();
    bool hasMesh0();
    bool hasMesh1();
    bool hasSkeleton();
};