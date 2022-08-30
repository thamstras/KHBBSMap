#include "BbsPmo.h"
#include <stdexcept>
#include "..\Utils\StreamUtils.h"
#include "..\Utils\BitCast.h"
#include "..\Utils\MagicCode.h"

constexpr uint32_t pmoMagic = MagicCode('P', 'M', 'O', '\0');
constexpr uint32_t bonMagic = MagicCode('B', 'O', 'N', '\0');



PmoHeader ParsePmoHeader(std::ifstream& file)
{
    PmoHeader header{};
    ReadStream(file, header.magic);
    if (header.magic != pmoMagic)
    {
        throw std::runtime_error("File is not a valid PMO file! (Magic code fail)");
    }
    for (int i = 0; i < 4; i++) ReadStream(file, header.unk_04[i]);
    ReadStream(file, header.textureCount);
    ReadStream(file, header.padding1);
    ReadStream(file, header.skeletonOffset);
    ReadStream(file, header.mesh0Offset);
    ReadStream(file, header.triangleCount);
    ReadStream(file, header.vertexCount);
    ReadStream(file, header.scale);
    ReadStream(file, header.mesh1Offset);
    for (int i = 0; i < 8 * 4; i++) ReadStream(file, header.boundingBox[i]);

    return header;
}

PmoTexture ParsePmoTexture(std::ifstream& file)
{
    PmoTexture tex{};
    ReadStream(file, tex.dataOffset); 
    file.read(tex.resourceName, 0xC);
    for (int i = 0; i < 4; i++) ReadStream(file, tex.unk_10[i]);
    return tex;
}

PmoVertexFormatFlags ParseVertexFormat(std::ifstream& file)
{
    uint32_t raw;
    PmoVertexFormatFlags format;
    ReadStream(file, raw);
    format = bit_cast<PmoVertexFormatFlags, uint32_t>(raw);
    return format;
}

PmoMeshHeader ParsePmoMeshHeader(std::ifstream& file, bool hasSkeleton)
{
    PmoMeshHeader header;
    ReadStream(file, header.vertexCount);
    ReadStream(file, header.textureIndex);
    ReadStream(file, header.vertexSize);
    header.vertexFormat = ParseVertexFormat(file);
    ReadStream(file, header.unk_08);
    ReadStream(file, header.triStripCount);
    ReadStream(file, header.attributes);
    if (hasSkeleton)
    {
        for (int j = 0; j < 8; j++) ReadStream(file, header.jointIndices[j]); 
    }
    else
    {
        for (int j = 0; j < 8; j++) header.jointIndices[j] = 0;
    }
    if (header.vertexFormat.diffuse)
    {
        ReadStream(file, header.diffuseColor);
    }
    else
    {
        header.diffuseColor = 0;
    }
    if (header.triStripCount != 0)
    {
        for (int s = 0; s < header.triStripCount; s++)
            header.triStripLengths.push_back(ReadStream<uint16_t>(file));
        
    }
    return header;
}

PmoMesh ParsePmoMesh(std::ifstream& file, bool hasSkeleton)
{
    PmoMesh mesh;
    mesh.header = ParsePmoMeshHeader(file, hasSkeleton);
    if (mesh.header.vertexCount > 0)
    {
        mesh.vertexData = ReadBlob(file, mesh.header.vertexCount * mesh.header.vertexSize);
    }
    return mesh;
}

PmoSkelHeader ParsePmoSkeletonHeader(std::ifstream& file)
{
    PmoSkelHeader header{};
    ReadStream(file, header.magic);
    if (header.magic != bonMagic)
    {
        throw std::runtime_error("File is not a valid PMO Skeleton! (Magic code fail)");
    }
    ReadStream(file, header.padding0);
    ReadStream(file, header.jointCount);
    ReadStream(file, header.padding1);
    ReadStream(file, header.asn_bone_max);
    ReadStream(file, header.nStdBone);
    return header;
}

PmoJoint ParsePmoJoint(std::ifstream& file)
{
    PmoJoint joint{};
    ReadStream(file, joint.index);
    ReadStream(file, joint.padding0);
    ReadStream(file, joint.parent);
    ReadStream(file, joint.padding1);
    ReadStream(file, joint.asnnum);
    ReadStream(file, joint.padding2);
    ReadStream(file, joint.padding3);
    file.read(joint.name, 0x10);
    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
            ReadStream(file, joint.transform[y][x]);
    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
            ReadStream(file, joint.transformInverse[y][x]);
    return joint;
}

PmoSkeleton ParsePmoSkeleton(std::ifstream& file)
{
    PmoSkeleton skele;
    skele.header = ParsePmoSkeletonHeader(file);
    for (uint32_t j = 0; j < skele.header.jointCount; j++)
        skele.joints.push_back(ParsePmoJoint(file));
    return skele;
}

PmoFile PmoFile::ReadPmoFile(std::ifstream& file, std::streamoff base)
{
    PmoFile pmo;
    pmo.header = ParsePmoHeader(file);
    if (pmo.header.textureCount != 0)
    {
        for (int i = 0; i < pmo.header.textureCount; i++)
            pmo.textures.push_back(ParsePmoTexture(file));
    }
    if (pmo.header.mesh0Offset != 0)
    {
        file.seekg(base + (std::streamoff)pmo.header.mesh0Offset, std::ios_base::beg);
        do
        {
            pmo.mesh0.push_back(ParsePmoMesh(file, (pmo.header.skeletonOffset != 0)));
            Realign(file, 0x4);
        } while (pmo.mesh0[pmo.mesh0.size() - 1].header.vertexCount != 0);
    }
    if (pmo.header.mesh1Offset != 0)
    {
        file.seekg(base + (std::streamoff)pmo.header.mesh1Offset, std::ios_base::beg);
        do
        {
            pmo.mesh1.push_back(ParsePmoMesh(file, (pmo.header.skeletonOffset != 0)));
            Realign(file, 0x4);
        } while (pmo.mesh1[pmo.mesh1.size() - 1].header.vertexCount != 0);
    }
    if (pmo.header.skeletonOffset != 0)
    {
        file.seekg(base + (std::streamoff)pmo.header.skeletonOffset, std::ios_base::beg);
        pmo.skeleton = ParsePmoSkeleton(file);
    }
    return pmo;
}

bool PmoFile::hasTextures()
{
    return (this->header.textureCount > 0);
}

bool PmoFile::hasMesh0()
{
    return (this->header.mesh0Offset > 0);
}

bool PmoFile::hasMesh1()
{
    return (this->header.mesh1Offset > 0);
}

bool PmoFile::hasSkeleton()
{
    return (this->header.skeletonOffset > 0);
}