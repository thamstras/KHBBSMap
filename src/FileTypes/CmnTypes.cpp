#include "CmnTypes.h"
#include "Utils\StreamUtils.h"

FVECTOR3 ParseFVector3(std::ifstream& file)
{
	FVECTOR3 v{};
	v.x = ReadStream<float>(file);
	v.y = ReadStream<float>(file);
	v.z = ReadStream<float>(file);
}

FVECTOR4 ParseFVector4(std::ifstream& file)
{
	FVECTOR4 v{};
	v.x = ReadStream<float>(file);
	v.y = ReadStream<float>(file);
	v.z = ReadStream<float>(file);
	v.w = ReadStream<float>(file);
}

ICOLOR ParseIColor(std::ifstream& file)
{
	ICOLOR v{};
	v.r = ReadStream<uint8_t>(file);
	v.g = ReadStream<uint8_t>(file);
	v.b = ReadStream<uint8_t>(file);
	v.a = ReadStream<uint8_t>(file);
}