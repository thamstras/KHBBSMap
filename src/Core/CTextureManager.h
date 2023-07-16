#pragma once
#include "..\Common.h"
#include "CTexture.h"
#include <unordered_map>

enum class TexType {
	Static,
	Map
};

struct TextureInfo {
	std::string name;
	int fileIndex;
	std::shared_ptr<CTexture> texObject;
	glm::vec2 uvScroll;
	std::shared_ptr<CTexture> hdTexObject;
};

class CTextureManager
{
private:
	std::unordered_map<std::string, TextureInfo> themap;

public:
	//CTextureManager();

	void AddTexture(TexType type, TextureInfo info);
	std::shared_ptr<CTexture> GetTexture(const std::string& name, bool useHd = false);
	bool HaveTexture(const std::string& name);
	void RemoveTexture(const std::string& name);
	//void ClearType(TexType type);
	void SetHdTexture(int fileIndex, std::shared_ptr<CTexture> hdTexture);
};

