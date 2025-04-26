#include "BbsPvd.h"
#include <stdexcept>
#include "..\Utils\StreamUtils.h"
#include "..\Utils\BitCast.h"
#include "..\Utils\MagicCode.h"

constexpr uint32_t pvdMagic = MagicCode('P', 'V', 'D', '\0');

PvdHeader ParsePvdHeader(std::ifstream& file)
{
	PvdHeader header{};
    ReadStream(file, header.magic);
    if (header.magic != pvdMagic)
    {
        throw std::runtime_error("File is not a valid PVD file! (Magic code fail)");
    }
    header.version = ReadStream<uint16_t>(file);
    header.padding0 = ReadStream<uint16_t>(file);
    header.padding1[0] = ReadStream<uint32_t>(file);
    header.padding1[1] = ReadStream<uint32_t>(file);
    return header;
}

PvdData ParsePvdData(std::ifstream& file)
{
    PvdData data{};
    for (int i = 0; i < 4; i++)
        data.fogColor[i] = ReadStream<uint8_t>(file);
    data.fogStart = ReadStream<float>(file);
    data.fogEnd = ReadStream<float>(file);
    data.nearClip = ReadStream<float>(file);
    data.farClip = ReadStream<float>(file);
    data.glare = ReadStream<float>(file);
    for (int i = 0; i < 4; i++)
        data.clearColor[i] = ReadStream<uint8_t>(file);
    data.flags = ReadStream<uint32_t>(file);
    data.fov = ReadStream<float>(file);
    for (int i = 0; i < 3; i++)
        data.worldOffset[i] = ReadStream<float>(file);
    data.camOffsetY = ReadStream<float>(file);
    data.camRotX = ReadStream<float>(file);
    data.camDist = ReadStream<float>(file);
    data.camRotY = ReadStream<float>(file);
    return data;
}

PvdFile PvdFile::ReadPvdFile(std::ifstream& file, std::streamoff base)
{
    PvdFile pvd{};
    pvd.header = ParsePvdHeader(file);
    pvd.data = ParsePvdData(file);
    return pvd;
}