#include "CFramebuffer.h"

CFramebuffer::CFramebuffer(GLuint width, GLuint height, int samples) :
	width(width), height(height),
	FBO(0), RBO(0), texColorBuffer(0),
	isMultiSampled(samples > 1), FBO2(0), RBO2(0), multiSampleBuffer(0)
{

	GLint old_fbo;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &old_fbo);

	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	if (isMultiSampled)
	{
		glGenFramebuffers(1, &FBO2);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO2);

		glGenTextures(1, &multiSampleBuffer);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multiSampleBuffer);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenRenderbuffers(1, &RBO2);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO2);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, multiSampleBuffer, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO2);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);

}

void CFramebuffer::ReleaseResources()
{
	if (isMultiSampled)
	{
		glDeleteFramebuffers(1, &FBO2);
		FBO2 = 0;
		glDeleteTextures(1, &multiSampleBuffer);
		multiSampleBuffer = 0;
		glDeleteRenderbuffers(1, &RBO2);
		RBO2 = 0;
	}

	glDeleteFramebuffers(1, &FBO);
	FBO = 0;
	glDeleteTextures(1, &texColorBuffer);
	texColorBuffer = 0;
	glDeleteRenderbuffers(1, &RBO);
	RBO = 0;
}

CFramebuffer::~CFramebuffer()
{
	ReleaseResources();
}

CFramebuffer::CFramebuffer(CFramebuffer&& other) noexcept :
	width(other.width), height(other.height),
	FBO(std::exchange(other.FBO, 0)), RBO(std::exchange(other.RBO, 0)),
	texColorBuffer(std::exchange(other.texColorBuffer, 0)),
	isMultiSampled(other.isMultiSampled),
	FBO2(std::exchange(other.FBO2, 0)), RBO2(std::exchange(other.RBO2, 0)),
	multiSampleBuffer(std::exchange(other.multiSampleBuffer, 0))
{}

CFramebuffer& CFramebuffer::operator=(CFramebuffer&& other) noexcept
{
	if (this == &other)
		return *this;

	ReleaseResources();

	width = other.width;
	height = other.height;
	FBO = std::exchange(other.FBO, 0);
	RBO = std::exchange(other.RBO, 0);
	texColorBuffer = std::exchange(other.texColorBuffer, 0);
	isMultiSampled = other.isMultiSampled;
	FBO2 = std::exchange(other.FBO2, 0);
	RBO2 = std::exchange(other.RBO2, 0);
	multiSampleBuffer = std::exchange(other.multiSampleBuffer, 0);

	return *this;
}

GLuint CFramebuffer::Width() const { return width; }
GLuint CFramebuffer::Height() const { return height; }

void CFramebuffer::Bind()
{
	if (isMultiSampled)
		glBindFramebuffer(GL_FRAMEBUFFER, FBO2);
	else
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glViewport(0, 0, width, height);
}

GLuint CFramebuffer::ResolveTexture()
{
	if (!isMultiSampled) return texColorBuffer;

	GLint old_fbo;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &old_fbo);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO2);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);

	return texColorBuffer;
}