#pragma once

#include "..\Common.h"

class FileBuffer
{
public:
	uint8_t* buf;
	size_t size;

	FileBuffer(uint8_t* bytes, size_t byteCount);
	~FileBuffer();
};

class FileLoader
{
	std::string baseDir;
public:
	FileLoader(std::string baseDir);
	~FileLoader();

	std::shared_ptr<FileBuffer> LoadFile(std::string path);
};