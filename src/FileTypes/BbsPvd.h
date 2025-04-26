#pragma once
#include <vector>
#include <fstream>
#include <cstdint>

struct PvdHeader
{
    uint32_t magic;
    uint16_t version;
    uint16_t padding0;
    uint32_t padding1[2];
};

struct PvdData
{
    uint8_t fogColor[4];
    float fogStart;
    float fogEnd;
    float nearClip;
    float farClip;
    float glare;
    uint8_t clearColor[4];
    uint32_t flags;
    float fov;
    float worldOffset[3];
    float camOffsetY;
    float camRotX;
    float camDist;
    float camRotY;
};

struct PvdFile
{
    PvdHeader header;
    PvdData data;

    static PvdFile ReadPvdFile(std::ifstream& file, std::streamoff base);
};