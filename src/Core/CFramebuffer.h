#include "Common.h"

class CFramebuffer
{
	GLuint width, height;
	GLuint FBO, RBO;
	GLuint texColorBuffer;
	
	
	bool isMultiSampled;
	GLuint FBO2, RBO2;
	GLuint multiSampleBuffer;

public:
	CFramebuffer(GLuint width, GLuint height, int samples = 1);
	~CFramebuffer();

	GLuint Width();
	GLuint Height();

	void Bind();
	GLuint ResolveTexture();
};