#include "CTextureManager.h"
#include <algorithm>

void CTextureManager::AddTexture(TexType type, TextureInfo info)
{
	// UNUSED: type

	themap.insert(std::make_pair(info.name, info));
}

bool CTextureManager::HaveTexture(const std::string& name)
{
	return themap.contains(name);
}

std::shared_ptr<CTexture> CTextureManager::GetTexture(const std::string& name, bool useHd)
{
	auto pair = themap.find(name);
	if (pair != themap.end())
	{
		auto& texInfo = pair->second;
		
		if (useHd && texInfo.hdTexObject)
			return texInfo.hdTexObject;

		return texInfo.texObject;
	}

	return std::shared_ptr<CTexture>();
}

void CTextureManager::SetHdTexture(int fileIndex, std::shared_ptr<CTexture> hdTexture)
{
	auto itr = std::find_if(themap.begin(), themap.end(), [fileIndex](auto& pair) { return pair.second.fileIndex == fileIndex; });
	if (itr != themap.end())
	{
		itr->second.hdTexObject = hdTexture;
	}
}