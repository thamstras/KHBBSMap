#pragma once
#include "Common.h"
#include "FileManager.h"
#include "Render.h"

class FormatReader
{
	static void ApplyPVD(std::string path, RenderContext& context, float* cameraZoom);
};