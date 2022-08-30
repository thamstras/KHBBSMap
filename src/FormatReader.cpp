#include "FormatReader.h"
#include <cstdio>

struct FVECTOR3 {
    float x, y, z;
};

union ICOLOR {
    uint32 c;
    struct {
        uint8 r;
        uint8 g;
        uint8 b;
        uint8 a;
    } v;
};

struct PVD_PARAM {
    ICOLOR sFogColor;
    float fFogNear;
    float fFogFar;
    float fDispNear;
    float fDispFar;
    float fGlare;
    ICOLOR sClearColor;
    uint32 iFlag;
    float fViewAngle;
    FVECTOR3 vBasePos;
    float fCamOfsY;
    float fCamRotX;
    float fCamDist;
    float fCamRotY;
};

struct PVD_HEAD {
    uint32 id;
    uint16 Ver;
    uint16 pad16[1];
    uint32 pad[2];
};

struct PVD_DATA {
    PVD_HEAD sHd;
    PVD_PARAM sParam;
};

void FormatReader::ApplyPVD(std::string path, RenderContext& context, float* cameraZoom)
{
	// TODO: better.
	std::FILE* file = std::fopen(path.c_str(), "rb");
    char* fileBuf = (char*)_aligned_malloc(sizeof(PVD_DATA), 4);
    if (fileBuf == nullptr)
    {
        // TODO: Message box
        return;
    }
    size_t read = std::fread(fileBuf, sizeof(PVD_DATA), 1, file);
    if (read < sizeof(PVD_DATA))
    {
        // TODO: Message box
        return;
    }
    PVD_DATA* pvd = (PVD_DATA*)fileBuf;
    if (memcmp(&(pvd->sHd.id), "PVD\0", 4) != 0)
    {
        // TODO: Message box
        return;
    }

    context.fogColor = glm::vec4((float)pvd->sParam.sFogColor.v.r / 255.0f,
        (float)pvd->sParam.sFogColor.v.g / 255.0f,
        (float)pvd->sParam.sFogColor.v.b / 255.0f,
        (float)pvd->sParam.sFogColor.v.a / 255.0f);
    context.fogNear = pvd->sParam.fFogNear;
    context.fogFar = pvd->sParam.fFogFar;
    context.render_nearClip = pvd->sParam.fDispNear;
    context.render_farClip = pvd->sParam.fDispFar;
    context.clearColor = glm::vec4((float)pvd->sParam.sClearColor.v.r / 255.0f,
        (float)pvd->sParam.sClearColor.v.g / 255.0f,
        (float)pvd->sParam.sClearColor.v.b / 255.0f,
        (float)pvd->sParam.sClearColor.v.a / 255.0f);
    if (cameraZoom) *cameraZoom = glm::degrees(pvd->sParam.fViewAngle);

    /*
	// fog color
	std::fseek(file, 0x10, SEEK_SET);
	glm::u8vec4 colorIn;
	std::fread((void*)(glm::value_ptr(colorIn)), sizeof(glm::u8), 4, file);
	context.fogColor = glm::vec4((float)colorIn.r / 255.0f, (float)colorIn.g / 255.0f, (float)colorIn.b / 255.0f, (float)colorIn.a / 255.0f);

	std::fread((void*)(&context.fogNear), sizeof(float), 1, file);
	std::fread((void*)(&context.fogFar), sizeof(float), 1, file);
	std::fread((void*)(&context.render_nearClip), sizeof(float), 1, file);
	std::fread((void*)(&context.render_farClip), sizeof(float), 1, file);

	// clear color
	std::fseek(file, 0x28, SEEK_SET);
	std::fread((void*)(glm::value_ptr(colorIn)), sizeof(glm::u8), 4, file);
	context.clearColor = glm::vec4((float)colorIn.r / 255.0f, (float)colorIn.g / 255.0f, (float)colorIn.b / 255.0f, (float)colorIn.a / 255.0f);

	std::fseek(file, 0x30, SEEK_SET);
	float viewIn;
	std::fread((void*)(&viewIn), sizeof(float), 1, file);
	//g_camera.Zoom = glm::degrees(viewIn);
	if (cameraZoom) *cameraZoom = glm::degrees(viewIn);
    */

    _aligned_free(fileBuf);
	std::fclose(file);
}