#include "Common.h"

class CFramebuffer
{
	GLuint width, height;
	GLuint FBO, RBO;
	GLuint texColorBuffer;
	
	
	bool isMultiSampled;
	GLuint FBO2, RBO2;
	GLuint multiSampleBuffer;

	void ReleaseResources();

public:
	CFramebuffer(GLuint width, GLuint height, int samples = 1);
	~CFramebuffer();

	CFramebuffer(const CFramebuffer& other) = delete;
	CFramebuffer& operator=(const CFramebuffer& other) = delete;

	CFramebuffer(CFramebuffer&& other) noexcept;
	CFramebuffer& operator=(CFramebuffer&& other) noexcept;

	GLuint Width() const;
	GLuint Height() const;

	void Bind();
	GLuint ResolveTexture();
};