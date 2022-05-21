#pragma once
#include <vector>
#include <fstream>
#include <cstdint>

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
	IT_RGBA = 3,
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

struct GsTex0
{
	uint64_t TBP0 : 14; // Texture Buffer Base Pointer (Address/256)
	uint64_t TBW : 6; // Texture Buffer Width (Texels/64)
	uint64_t PSM : 6; // Pixel Storage Format (0 = 32bit RGBA)
	uint64_t TW : 4; // width = 2^TW
	uint64_t TH : 4; // height = 2^TH
	uint64_t TCC : 1; // 0 = RGB, 1 = RGBA
	uint64_t TFX : 2; // TFX  - Texture Function (0=modulate, 1=decal, 2=hilight, 3=hilight2)
	uint64_t CBP : 14; // CLUT Buffer Base Pointer (Address/256)
	uint64_t CPSM : 4; // CLUT Storage Format
	uint64_t CSM : 1; // CLUT Storage Mode
	uint64_t CSA : 5; // CLUT Offset
	uint64_t CLD : 3; // CLUT Load Control
};

struct Tm2FileHeader
{
	uint32_t magic;
	uint8_t version;
	uint8_t format;
	uint16_t pictureCount;
	uint32_t padding[2];
};

struct Tm2PictureHeader
{
	uint32_t totalSize;
	uint32_t clutSize;
	uint32_t imageSize;
	uint16_t headerSize;
	uint16_t clutColors;
	uint8_t  format;
	uint8_t  mipMapCount;
	uint8_t  clutType;
	uint8_t  imageType;
	uint16_t width;
	uint16_t height;
	GsTex0 gsTex0b;
	GsTex0 gsTex1b;
	uint32_t gsRegs;
	uint32_t gsTexClut;
};

struct Tm2Mipmap
{
	uint32_t miptbp0;
	uint32_t miptbp1;
	uint32_t miptbp2;
	uint32_t miptbp3;
	uint32_t sizes[8];
};

struct Tm2Picture
{
	Tm2PictureHeader header;
	Tm2Mipmap mipmap;
	std::vector<uint8_t> pixelData;
	std::vector<uint8_t> clutData;
};

class Tm2File
{
public:
	Tm2FileHeader header;
	std::vector<Tm2Picture> pictures;

	uint16_t pictureCount();

	static Tm2File ReadTm2File(std::ifstream& file, std::streamoff base = 0);
};