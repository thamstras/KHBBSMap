#include "CTextureObject.h"

using namespace BBS;

CTextureObject::CTextureObject()
{
	texture = nullptr;
	clutType = CLT_TYPE::CT_NONE;
	clutCount = 0;
	imageType = IMG_TYPE::IT_RGBA;
	imageWidth = 0;
	imageHeight = 0;
	textureWidth = 0;
	textureHeight = 0;
}

CTextureObject::~CTextureObject()
{
	if (texture)
	{
		texture->ogl_unload();
		delete texture;
		texture = nullptr;
	}
}

void CTextureObject::LoadTM2(Tm2File& tm2)
{
	Tm2Picture& picture = tm2.pictures.at(0);
	this->clutType = (CLT_TYPE)picture.header.clutType;
	this->clutCount = picture.header.clutColors;
	this->clut = std::vector<uint8>(picture.clutData);

	this->imageType = (IMG_TYPE)picture.header.imageType;
	this->imageWidth = picture.header.width;
	this->imageHeight = picture.header.height;
	this->image = std::vector<uint8>(picture.pixelData);

	this->textureWidth = 1 << picture.header.gsTex0b.TW;
	this->textureHeight = 1 << picture.header.gsTex0b.TH;
}

void CTextureObject::CreateTexture()
{
	uint32 width = imageWidth;
	uint32 height = imageHeight;

	uint32 psWidth = textureWidth;
	uint32 psHeight = textureHeight;

	std::vector<uint8> decodedData = std::vector<uint8>();
	decodedData.reserve(4 * width * height);

	uint8* pixelData = image.data();
	int pixelPtr = 0;
	uint8* clutData = clut.data();
	uint32 rem = (psWidth - width) * 4;

	for (int py = 0; py < height; py++)
	{
		for (int px = 0; px < width; px++)
		{
			// STEP 1: Decode clut index
			uint32 pixelIndex0 = pixelData[pixelPtr];
			uint32 pixelIndex1 = pixelIndex0;

			switch (imageType)
			{
			case IT_RGBA:	// No CLUT.
				break;
			case IT_CLUT4:	// 4 bit CLUT => 2 pixels per byte
				pixelIndex0 &= 0x0F;
				pixelIndex1 = (pixelIndex1 >> 4) & 0x0F;
				pixelPtr++;
				break;
			case IT_CLUT8:	// 8 bit CLUT
				if ((pixelIndex0 & 31) >= 8)
				{
					if ((pixelIndex0 & 31) < 16)
					{
						pixelIndex0 += 8;				// +8 - 15 to +16 - 23
					}
					else if ((pixelIndex0 & 31) < 24)
					{
						pixelIndex0 -= 8;				// +16 - 23 to +8 - 15
					}
				}
				pixelPtr++;
				break;
			default:
				// TODO: Error
				break;
			}

			// STEP 2: Decode first pixel from data/clut
			uint16 temp16;
			switch (clutType)
			{
			case CT_NONE:
				decodedData.push_back(pixelData[pixelPtr++]);
				decodedData.push_back(pixelData[pixelPtr++]);
				decodedData.push_back(pixelData[pixelPtr++]);
				decodedData.push_back(pixelData[pixelPtr++]);
				break;
			case CT_A1BGR5:
				pixelIndex0 *= 2;
				temp16 = clutData[pixelIndex0 + 0] | (clutData[pixelIndex0 + 1] << 8);
				decodedData.push_back((temp16 & 0x1F) * (1.0 / 31.0 * 255.0));
				decodedData.push_back(((temp16 >> 5) & 0x1F) * (1.0 / 31.0 * 255.0));
				decodedData.push_back(((temp16 >> 10) & 0x1F) * (1.0 / 31.0 * 255.0));
				decodedData.push_back((temp16 & 0x8000) ? (0xFF) : (0));
				break;
			case CT_XBGR8:
				pixelIndex0 *= 3;
				decodedData.push_back(clutData[pixelIndex0 + 0]);
				decodedData.push_back(clutData[pixelIndex0 + 1]);
				decodedData.push_back(clutData[pixelIndex0 + 2]);
				decodedData.push_back(0xFF);
				break;
			case CT_ABGR8:
				pixelIndex0 *= 4;
				decodedData.push_back(clutData[pixelIndex0 + 0]);
				decodedData.push_back(clutData[pixelIndex0 + 1]);
				decodedData.push_back(clutData[pixelIndex0 + 2]);
				decodedData.push_back(clutData[pixelIndex0 + 3]);
				break;
			default:
				// TODO: Error
				break;
			}

			// STEP 3: handle possible 2nd pixel
			if (imageType == IT_CLUT4)
			{
				px++;
				switch (clutType)
				{
					// NOTE: No CT_NONE, as imageType == IT_CLUT4 && clutType == CT_NONE would be invalid
				case CT_A1BGR5:
					pixelIndex1 *= 2;
					temp16 = clutData[pixelIndex1 + 0] | (clutData[pixelIndex1 + 1] << 8);
					decodedData.push_back((temp16 & 0x1F) * (1.0 / 31.0 * 255.0));
					decodedData.push_back(((temp16 >> 5) & 0x1F) * (1.0 / 31.0 * 255.0));
					decodedData.push_back(((temp16 >> 10) & 0x1F) * (1.0 / 31.0 * 255.0));
					decodedData.push_back((temp16 & 0x8000) ? (0xFF) : (0));
					break;
				case CT_XBGR8:
					pixelIndex1 *= 3;
					decodedData.push_back(clutData[pixelIndex1 + 0]);
					decodedData.push_back(clutData[pixelIndex1 + 1]);
					decodedData.push_back(clutData[pixelIndex1 + 2]);
					decodedData.push_back(0xFF);
					break;
				case CT_ABGR8:
					pixelIndex1 *= 4;
					decodedData.push_back(clutData[pixelIndex1 + 0]);
					decodedData.push_back(clutData[pixelIndex1 + 1]);
					decodedData.push_back(clutData[pixelIndex1 + 2]);
					decodedData.push_back(clutData[pixelIndex1 + 3]);
					break;
				default:
					// TODO: Error
					break;
				}
			}
		}

		for (int p = 0; p < rem; p++)
			decodedData.push_back(0);
	}

	texture = new CTexture(textureWidth, textureHeight, decodedData.data(), PF_RGBA32);
}