#pragma once
#include "Common.h"
#include <cstdint>

enum PixelFormat
{
	PF_RGBA32
};

size_t PixelFormatSize(PixelFormat pf);

class Texture
{
public:
	Texture(uint32_t width, uint32_t height, uint8_t *data, PixelFormat format);
	~Texture();

	void ogl_loadIfNeeded();
	void ogl_unload();

	uint32_t getWidth() const;
	uint32_t getHeight() const;

	GLuint getOglId() const;

	bool isLoaded() const;

private:
	unsigned int width, height;
	uint8_t *data;
	GLuint ogl_texid;
	bool ogl_loaded;
};