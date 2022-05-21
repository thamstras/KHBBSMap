#pragma once
#include "..\Common.h"
#include "CoreRender.h"

struct WorldContext
{
	float deltaTime;
	double worldTime;
	long frameCount;

	std::string mapName;
	// std::vector<something> loadedFiles;
	// some way of accessing the scene's contents

	RenderContext* render;
};