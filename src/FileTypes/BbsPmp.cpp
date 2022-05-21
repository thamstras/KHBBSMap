#include "BbsPmp.h"
#include <stdexcept>
#include "..\Utils\MagicCode.h"
#include "..\Utils\StreamUtils.h"

constexpr uint32_t pmpMagic = MagicCode('P', 'M', 'P', '\0');

PmpHeader ParsePmpHeader(std::ifstream& file)
{
    PmpHeader header{};
    header.magic = ReadStream<uint32_t>(file);
    if (header.magic != pmpMagic)
    {
        throw std::runtime_error("File is not a valid PMP file! (magic code fail)");
    }
    header.version = ReadStream<uint16_t>(file);
    header.padding0 = ReadStream<uint16_t>(file);
    header.padding1 = ReadStream<uint32_t>(file);
    ReadStreamArr<uint8_t>(file, header.padding2, 3);
    header.flag = ReadStream<uint8_t>(file);
    header.instance_count = ReadStream<uint16_t>(file);
    header.model_count = ReadStream<uint16_t>(file);
    header.padding3 = ReadStream<uint32_t>(file);
    header.padding4 = ReadStream<uint16_t>(file);
    header.tex_count = ReadStream<uint16_t>(file);
    header.tex_list_offset = ReadStream<uint32_t>(file);
    return header;
};

PmpInstance ParsePmpInstance(std::ifstream& file)
{
    PmpInstance entry;
    for (int i = 0; i < 3; i++) entry.loc[i] = ReadStream<float>(file);
    for (int i = 0; i < 3; i++) entry.rot[i] = ReadStream<float>(file);
    for (int i = 0; i < 3; i++) entry.scale[i] = ReadStream<float>(file);
    entry.offset = ReadStream<uint32_t>(file);
    entry.padding0 = ReadStream<uint32_t>(file);
    entry.flags = ReadStream<uint16_t>(file);
    entry.model_idx = ReadStream<uint16_t>(file);
    return entry;
}

PmpTexEntry ParsePmpTexEntry(std::ifstream& file)
{
    PmpTexEntry entry;
    entry.offset = ReadStream<uint32_t>(file);
    file.read(entry.name, 0xC);
    entry.scrollX = ReadStream<float>(file);
    entry.scrollY = ReadStream<float>(file);
    ReadStreamArr<uint32_t>(file, entry.padding0, 2);
    return entry;
}

PmpFile PmpFile::ReadPmpFile(std::ifstream& file, std::streamoff base)
{
    PmpFile pmp;
    pmp.header = ParsePmpHeader(file);
    for (int o = 0; o < pmp.header.instance_count; o++)
        pmp.instances.push_back(ParsePmpInstance(file));
    file.seekg(base + (std::streamoff)pmp.header.tex_list_offset);
    for (int t = 0; t < pmp.header.tex_count; t++)
        pmp.textures.push_back(ParsePmpTexEntry(file));
    for (PmpInstance& entry : pmp.instances)
    {
        if (entry.offset != 0)
        {
            file.seekg(base + (std::streamoff)entry.offset);
            entry.data = PmoFile::ReadPmoFile(file, base);
        }
    }
    for (PmpTexEntry& entry : pmp.textures)
    {
        file.seekg(base + (std::streamoff)entry.offset);
        entry.data = Tm2File::ReadTm2File(file, base);
    }
    return pmp;
}

uint16_t PmpFile::instanceCount()
{
    return header.instance_count;
}

uint16_t PmpFile::textureCount()
{
    return header.tex_count;
}