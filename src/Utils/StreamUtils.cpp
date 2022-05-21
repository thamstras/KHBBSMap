#include "StreamUtils.h"

std::vector<uint8_t> ReadBlob(std::istream& stream, size_t size)
{
	if (!stream.good())
	{
		if (stream.eof())
			throw std::runtime_error("Unexpected EOF");
		else
			throw std::runtime_error("Unknown IO Error");
	}
	std::vector<uint8_t> data = std::vector<uint8_t>(size);
	for (size_t i = 0; i < size; i++)
	{
		data.push_back(stream.get());
		if (!stream.good())
		{
			if (stream.eof())
				throw std::runtime_error("Unexpected EOF");
			else
				throw std::runtime_error("Unknown IO Error");
		}
	}
	return data;
}

void Realign(std::istream& stream, size_t size)
{
	std::streamoff pos = stream.tellg();
	if (pos % size != 0)
	{
		stream.seekg((size - (pos % size)), std::ios_base::cur);
	}
}