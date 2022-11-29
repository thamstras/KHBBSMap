#pragma once
#include "Common.h"
#include <cstdint>

enum PixelFormat
{
	PF_RGBA32
};

size_t PixelFormatSize(PixelFormat pf);

class CTexture
{
public:
	CTexture(uint32_t width, uint32_t height, uint8_t *data, PixelFormat format, void* userPtr = nullptr);
	virtual ~CTexture();

	CTexture(const CTexture& other) = delete;
	CTexture& operator=(const CTexture& other) = delete;

	CTexture(CTexture&& other) noexcept;
	CTexture& operator=(CTexture&& other) noexcept;

	void ogl_loadIfNeeded();
	void ogl_unload();

	uint32_t getWidth() const;
	uint32_t getHeight() const;
	const uint8_t* getPixels() const;

	GLuint getOglId() const;

	bool isLoaded() const;

	void* userPtr;

private:
	unsigned int width, height;
	uint8_t *data;
	GLuint ogl_texid;
	bool ogl_loaded;
};