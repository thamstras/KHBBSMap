#include "Tim2.h"
#include <stdexcept>
#include "..\Utils\MagicCode.h"
#include "..\Utils\BitCast.h"
#include "..\Utils\StreamUtils.h"

constexpr uint32_t tim2Magic = MagicCode('T', 'I', 'M', '2');

Tm2FileHeader ParseTm2FileHeader(std::ifstream& file)
{
	Tm2FileHeader header{};
	ReadStream(file, header.magic);
	if (header.magic != tim2Magic)
	{
		throw std::runtime_error("File is not a valid TM2 file! (Magic code fail)");
	}
	ReadStream(file, header.version);
	ReadStream(file, header.format);
	ReadStream(file, header.pictureCount);
	ReadStream(file, header.padding[0]);
	ReadStream(file, header.padding[1]);
	return header;
}

GsTex0 ParseGSTex(std::ifstream& file)
{
	uint64_t raw;
	GsTex0 reg;
	ReadStream(file, raw);
	reg = bit_cast<GsTex0, uint64_t>(raw);
	return reg;
}

Tm2PictureHeader ParseTm2PictureHeader(std::ifstream& file)
{
	Tm2PictureHeader header{};
	ReadStream(file, header.totalSize);
	ReadStream(file, header.clutSize);
	ReadStream(file, header.imageSize);
	ReadStream(file, header.headerSize);
	ReadStream(file, header.clutColors);
	ReadStream(file, header.format);
	ReadStream(file, header.mipMapCount);
	ReadStream(file, header.clutType);
	ReadStream(file, header.imageType);
	ReadStream(file, header.width);
	ReadStream(file, header.height);
	header.gsTex0b = ParseGSTex(file);
	header.gsTex1b = ParseGSTex(file);
	ReadStream(file, header.gsRegs);
	ReadStream(file, header.gsTexClut);
	return header;
}

Tm2Mipmap ParseTm2MipMap(std::ifstream& file)
{
	Tm2Mipmap mipmap{};
	ReadStream(file, mipmap.miptbp0);
	ReadStream(file, mipmap.miptbp1);
	ReadStream(file, mipmap.miptbp2);
	ReadStream(file, mipmap.miptbp3);
	for (int i = 0; i < 8; i++) ReadStream(file, mipmap.sizes[i]);
	return mipmap;
}

Tm2Picture ParseTm2Picture(std::ifstream& file)
{
	Tm2Picture picture;
	//std::cout << "PICTURE START " << file.tellg() << std::endl;
	picture.header = ParseTm2PictureHeader(file);
	//std::cout << "TOTAL SIZE " << picture.header.totalSize << std::endl;
	//std::cout << "HEADER SIZE " << picture.header.headerSize << std::endl;
	//std::cout << "PICTURE SIZE " << picture.header.imageSize << std::endl;
	//std::cout << "CLUT SIZE " << picture.header.clutSize << std::endl;
	//std::cout << "PICTURE HEADER END " << file.tellg() << std::endl;
	if (picture.header.mipMapCount > 1)
		picture.mipmap = ParseTm2MipMap(file);
	//std::cout << "PIXEL START " << file.tellg() << std::endl;
	picture.pixelData = ReadBlob(file, picture.header.imageSize);
	//std::cout << "CLUT START " << file.tellg() << std::endl;
	picture.clutData = ReadBlob(file, 4 * picture.header.clutColors);
	//std::cout << "PICTURE END " << file.tellg() << std::endl;
	return picture;
}

Tm2File Tm2File::ReadTm2File(std::ifstream& file, std::streamoff base)
{
	Tm2File tm2;
	tm2.header = ParseTm2FileHeader(file);
	for (int p = 0; p < tm2.header.pictureCount; p++)
		tm2.pictures.push_back(ParseTm2Picture(file));
	return tm2;
}

uint16 Tm2File::pictureCount()
{
	return header.pictureCount;
}