#pragma once

#include "..\Common.h"
#include "CMesh.h"
#include "CTexture.h"
#include "CoreRender.h"

class CIndexedMesh
{
public:
	std::vector<float> vertData;
	std::vector<unsigned int> indexData;
	unsigned int vertCount;
	unsigned int polyCount;
};