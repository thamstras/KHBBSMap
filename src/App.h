#pragma once
#include "Common.h"
#include <memory>

#include "BBS/CScene.h"
#include "BBS/CCollision.h"
#include "Core/CFramebuffer.h"

class App
{
public:
	std::string loadedPmpFile;
	std::unique_ptr<BBS::CScene> Map;

	std::string loadedPmoFile;
	std::unique_ptr<BBS::CCollision> Collision;

	std::unique_ptr<CFramebuffer> ViewportFramebuffer;
};