#include "Texture.h"

size_t PixelFormatSize(PixelFormat pf)
{
	switch (pf)
	{
	case PF_RGBA32:
		return 4;
	}

	return -1; // This will def cause an alloc error.
}

Texture::Texture(uint32_t width, uint32_t height, uint8_t *data, PixelFormat format, void* userPtr)
{
	this->width = width;
	this->height = height;
	int pxSiz = PixelFormatSize(format);
	int rowLen = width * pxSiz;
	this->data = (uint8_t *)malloc(width * height * pxSiz);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			for (int z = 0; z < pxSiz; z++)
			{
				this->data[y * rowLen + x * pxSiz + z] = data[y * rowLen + x * pxSiz + z];
			}
		}
	}

	ogl_texid = 0;
	ogl_loaded = false;

	this->userPtr = userPtr;
}

Texture::~Texture()
{
	free(this->data);
}

void Texture::ogl_loadIfNeeded()
{
	if (ogl_loaded) return;

	glGenTextures(1, &ogl_texid);
	glBindTexture(GL_TEXTURE_2D, ogl_texid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	ogl_loaded = true;
}

void Texture::ogl_unload()
{
	glDeleteTextures(1, &ogl_texid);
	ogl_loaded = false;
}

uint32_t Texture::getWidth() const
{
	return width;
}

uint32_t Texture::getHeight() const
{
	return height;
}

const uint8_t* Texture::getPixels() const
{
	return (data);
}

GLuint Texture::getOglId() const
{
	return ogl_texid;
}

bool Texture::isLoaded() const
{
	return ogl_loaded;
}
