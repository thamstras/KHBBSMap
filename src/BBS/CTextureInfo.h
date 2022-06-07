#pragma once
#include "Common.h"
#include "CTextureObject.h"
#include "FileTypes/BbsPmp.h"

namespace BBS
{

	class CTextureInfo
	{
	public:
		CTextureObject* srcTexture;
		std::string name;
		float scrollSpeed_x, scrollSpeed_y;
		glm::vec2 currentScroll;

		CTextureInfo(PmpTexEntry& texInfo, CTextureObject* texObj);
		~CTextureInfo();
		void Update(float deltaTime, double worldTime);
	};

}