#pragma once
#include "..\Common.h"
#include "..\Core\CMesh.h"
#include "..\Core\CTexture.h"
#include "CoreRender.h"

class CMesh;

/*enum VERTEX_ATTRIBUTE
{
	ATTRIB_POSITION,
	ATTRIB_NORMAL0,
	ATTRIB_COLOR,
	ATTRIB_UV0,
	ATTRIB_W0
};*/

struct CMeshSection
{
	unsigned int vertCount;
	unsigned int textureIndex;
	unsigned int stride;
	GLenum primType;
	bool twoSided;
	bool blend;
	std::vector<unsigned int> kickList;
};

class CMesh
{
private:
	static CTexture* dummyTexture;
	static glm::vec2 zeroOffset;

	std::shared_ptr<CShader> SelectShader(RenderContext& context);

public:
	CMesh();
	~CMesh();
	void Build();
	void UnBuild();

	std::vector<CMeshSection> sections;
	std::vector<CTexture*> textures;
	std::vector<glm::vec2> uvOffsets;
	std::vector<float> vertData;
	//std::vector<VERTEX_ATTRIBUTE> vertAttribs;
	unsigned int vertCount;
	unsigned int polyCount;

	glm::vec3 Pos, Rot, Scale;

	GLuint VAO, VBO;

	void Draw(RenderContext& context);
	void Draw(RenderContext& context, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
};