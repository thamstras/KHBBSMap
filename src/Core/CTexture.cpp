#include "CTexture.h"

size_t PixelFormatSize(PixelFormat pf)
{
	switch (pf)
	{
	case PF_RGBA32:
		return 4;
	}

	return -1; // This will def cause an alloc error.
}

CTexture::CTexture(uint32_t width, uint32_t height, uint8_t *data, PixelFormat format, void* userPtr)
{
	if (width == 0 || height == 0 || data == nullptr)
		throw std::exception("Tried to create invalid texture");

	this->width = width;
	this->height = height;
	int pxSiz = PixelFormatSize(format);
	int rowLen = width * pxSiz;
	//this->data = (uint8_t *)malloc(width * height * pxSiz);
	this->data = new uint8_t[width * height * pxSiz];

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

CTexture::~CTexture()
{
	if (ogl_loaded) ogl_unload();
	delete[] this->data;
}

CTexture::CTexture(CTexture&& other) noexcept :
	width(std::exchange(other.width, 0)), height(std::exchange(other.height, 0)),
	data(std::exchange(other.data, nullptr)),
	ogl_texid(std::exchange(other.ogl_texid, 0)), ogl_loaded(std::exchange(other.ogl_loaded, false)),
	userPtr(std::exchange(other.userPtr, nullptr))
{}

CTexture& CTexture::operator=(CTexture&& other) noexcept
{
	if (this == &other)
		return *this;

	if (ogl_loaded) ogl_unload();
	free(data);

	width = std::exchange(other.width, 0);
	height = std::exchange(other.height, 0);
	data = std::exchange(other.data, nullptr);
	ogl_texid = std::exchange(other.ogl_texid, 0);
	ogl_loaded = std::exchange(other.ogl_loaded, false);

	return *this;
}

void CTexture::ogl_loadIfNeeded()
{
	if (ogl_loaded) return;

	glGenTextures(1, &ogl_texid);
	glBindTexture(GL_TEXTURE_2D, ogl_texid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	ogl_loaded = true;
}

void CTexture::ogl_unload()
{
	glDeleteTextures(1, &ogl_texid);
	ogl_loaded = false;
}

uint32_t CTexture::getWidth() const
{
	return width;
}

uint32_t CTexture::getHeight() const
{
	return height;
}

const uint8_t* CTexture::getPixels() const
{
	return (data);
}

GLuint CTexture::getOglId() const
{
	return ogl_texid;
}

bool CTexture::isLoaded() const
{
	return ogl_loaded;
}
