#pragma once
#include "Common.h"
#include "Core\CoreRender.h"
#include "Core\CCamera.h"
#include "Core\World.h"
#include "CMap.h"

namespace BBS
{
	// Everything that goes inside the "Scene" window
	class CScene
	{
		void StartFrame();

	public:
		CMap* theMap;	// All the map geometry. PMP

		//CCollision* theCollision	// BCD

		//std::vector<CSpawnedObject*>	// OLO/PTX?

		RenderContext renderContext;
		WorldContext worldContext;
		CCamera camera;
		long frameCount;

		void Init();
		void Tick(float, double);
		void Draw();

		void ProcessKeyboard(GLFWwindow *window);
		void ProcessMouse(float deltaX, float deltaY);
	};
}