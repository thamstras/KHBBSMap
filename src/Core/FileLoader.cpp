#include "FileLoader.h"
#include <filesystem>

FileBuffer::FileBuffer(uint8_t* bytes, size_t byteCount) : buf(bytes), size(byteCount) {}

FileBuffer::~FileBuffer()
{
	if (buf)
	{
		_aligned_free(buf);
		buf = nullptr;
	}
	size = 0;
}

FileLoader::FileLoader(std::string baseDir) : baseDir(baseDir) {}

FileLoader::~FileLoader() {}

std::shared_ptr<FileBuffer> FileLoader::LoadFile(std::string path)
{
	std::filesystem::path fsPath = std::filesystem::path(path);
	if (!fsPath.is_absolute())
	{
		fsPath = std::filesystem::path(baseDir);
		fsPath /= path;
	}
	fsPath = fsPath.make_preferred();
	
	FILE* fp = fopen(fsPath.string().c_str(), "rb");
	if (!fp)
	{
		//LogError(__FUNCTION__, "Couldn't open filepath %s!", filePath);
		return std::shared_ptr<FileBuffer>();
	}

	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	uint8_t* fileBuffer = (uint8_t*)_aligned_malloc(fsize, 0x10);	// TODO: alignment
	if (!fileBuffer)
	{
		//LogError
		fclose(fp);
		return std::shared_ptr<FileBuffer>();
	}

	long read = fread(fileBuffer, 1, fsize, fp);
	if (read < fsize)
	{
		//LogError
		_aligned_free(fileBuffer);
		fclose(fp);
		return std::shared_ptr<FileBuffer>();
	}
}