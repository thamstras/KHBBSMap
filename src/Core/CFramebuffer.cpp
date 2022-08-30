#include "CFramebuffer.h"

CFramebuffer::CFramebuffer(GLuint width, GLuint height, int samples)
{
	this->width = width;
	this->height = height;

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

	if (samples > 1)
	{
		isMultiSampled = true;

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
	else
	{
		isMultiSampled = false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);

}

CFramebuffer::~CFramebuffer()
{
	if (isMultiSampled)
	{
		glDeleteFramebuffers(1, &FBO2);
		glDeleteTextures(1, &multiSampleBuffer);
		glDeleteRenderbuffers(1, &RBO2);
	}

	glDeleteFramebuffers(1, &FBO);
	glDeleteTextures(1, &texColorBuffer);
	glDeleteRenderbuffers(1, &RBO);
}

GLuint CFramebuffer::Width() { return width; }
GLuint CFramebuffer::Height() { return height; }

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