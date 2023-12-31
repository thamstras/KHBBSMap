#include "BbsBcd.h"
#include <stdexcept>
#include "..\Utils\MagicCode.h"
#include "..\Utils\StreamUtils.h"

constexpr uint32_t bcdMagic = MagicCode('@', 'b', 'c', 'd');

BcdHeader ParseBcdHeader(std::ifstream& file)
{
	BcdHeader header {};
	header.magic = ReadStream<uint32_t>(file);
	if (header.magic != bcdMagic)
	{
		throw std::runtime_error("File is not a valid BCD file! (magic code fail)");
	}
	header.version = ReadStream<uint32_t>(file);
	header.dataCount = ReadStream<int32_t>(file);
	header.dataOffset = ReadStream<uint32_t>(file);
	return header;
}

BcdData ParseBcdData(std::ifstream& file)
{
	BcdData data{};
	data.minX = ReadStream<float>(file);
	data.minY = ReadStream<float>(file);
	data.minZ = ReadStream<float>(file);
	data.maxX = ReadStream<float>(file);
	data.maxY = ReadStream<float>(file);
	data.maxZ = ReadStream<float>(file);
	data.cellSize = ReadStream<float>(file);
	data.flags = ReadStream<uint16_t>(file);
	data.divX = ReadStream<int16_t>(file);
	data.divY = ReadStream<int16_t>(file);
	data.divZ = ReadStream<int16_t>(file);
	data.vertCount = ReadStream<int16_t>(file);
	data.planeCount = ReadStream<int16_t>(file);
	data.vertexOffset = ReadStream<uint32_t>(file);
	data.planeOffset = ReadStream<uint32_t>(file);
	data.colorOffset = ReadStream<uint32_t>(file);
	data.bitArrayXOffset = ReadStream<uint32_t>(file);
	data.bitArrayYOffset = ReadStream<uint32_t>(file);
	data.bitArrayZOffset = ReadStream<uint32_t>(file);
	return data;
}

BcdPlane ParseBcdPlane(std::ifstream& file)
{
	BcdPlane plane{};
	ReadStreamArr<float>(file, plane.vPlane, 4);
	ReadStreamArr<uint16_t>(file, plane.vertIdx, 4);
	ReadStreamArr<uint16_t>(file, plane.colorIdx, 4);
	plane.info = ReadStream<int32_t>(file);
	plane.attrib = std::bit_cast<ATTR_BIT>(ReadStream<uint32_t>(file));
	ReadStreamArr<uint32_t>(file, plane.pad, 2);
	return plane;
}

BcdFile BcdFile::ReadBcdFile(std::ifstream& file, std::streamoff base)
{
	BcdFile bcd;

	bcd.header = ParseBcdHeader(file);

	file.seekg(base + (std::streamoff)bcd.header.dataOffset);
	for (int i = 0; i < bcd.header.dataCount; i++)
		bcd.data.push_back(ParseBcdData(file));
	
	for (BcdData& data : bcd.data)
	{
		file.seekg(base + (std::streamoff)data.vertexOffset);
		for (int v = 0; v < data.vertCount; v++)
			data.verts.push_back(ParseFVector4(file));
		
		file.seekg(base + (std::streamoff)data.planeOffset);
		uint16_t colors = 0;
		for (int p = 0; p < data.planeCount; p++)
		{
			data.planes.push_back(ParseBcdPlane(file));
			const BcdPlane& plane = data.planes.back();
			for (int v = 0; v < 4; v++)
				colors = std::max(colors, plane.colorIdx[v]);
		}
		
		file.seekg(base + (std::streamoff)data.colorOffset);
		for (int c = 0; c < colors; c++)
			data.colors.push_back(ParseIColor(file));

		// Round numPlanes up to next multiple of 8, divide by 8 and round up to the next multiple of 4.
		// (It's basically the number of int32s needed to have the number of bits needed to encode 1 bit per plane.)
		uint32_t bitmul = (data.planeCount + 7) & ~7;
		bitmul /= 8;
		bitmul = (bitmul + 3) & ~3;
		file.seekg(base + (std::streamoff)data.bitArrayXOffset);
		// TODO: Read bit array [bitmul * data.divX]
		file.seekg(base + (std::streamoff)data.bitArrayYOffset);
		// TODO: Read bit array [bitmul * data.divY]
		file.seekg(base + (std::streamoff)data.bitArrayZOffset);
		// TODO: Read bit array [bitmul * data.divZ]
	}

	return bcd;
}

bool BcdPlane::IsQuad()
{
	return vertIdx[3] != UINT16_MAX;
}