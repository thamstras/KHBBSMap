#include "CTextureInfo.h"

using namespace BBS;

CTextureInfo::CTextureInfo(PmpTexEntry& texInfo, CTextureObject* texObj)
{
	srcTexture = texObj;
	name = std::string(texInfo.name, 12);
	scrollSpeed_x = texInfo.scrollX;
	scrollSpeed_y = texInfo.scrollY;
	currentScroll = glm::vec2(0.0f);
}

CTextureInfo::~CTextureInfo()
{
	if (srcTexture)
	{
		delete srcTexture;
		srcTexture = nullptr;
	}
}

void CTextureInfo::Update(float deltaTime, double worldTime)
{
	currentScroll.x = worldTime * scrollSpeed_x * 30.f;
	currentScroll.y = worldTime * scrollSpeed_y * 30.f;
}