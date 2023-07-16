#pragma once
#include <string>
#include <filesystem>

class FileManager
{
public:
	FileManager();

	std::string GetFontPath(const std::string& fontName);
	std::string GetShaderPath(const std::string& shaderName);

	void GetFontCPath(const std::string& fontName, char* buffer, size_t buffer_len);
	void GetShaderCPath(const std::string& shaderName, char* buffer, size_t buffer_len);

	bool OpenFileWindow(std::string& out_filePath);
	bool GetExportFolder(std::string& out_path);

	void SetBBSRoot(const std::string& root_path);
	bool LoadBBSFile(const std::string& file);
	void* GetBBSResource(const std::string& name);

private:
	std::filesystem::path rootPath;
	std::filesystem::path resourcesPath;
};