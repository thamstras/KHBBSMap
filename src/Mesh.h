#pragma once
#include "Common.h"
#include "Core\CShader.h"
#include <cstdint>
#include "Core\CTexture.h"
#include "Render.h"
#include "BBSTypes.h"

struct OmniVert
{
	float x, y, z, w;
	float r, g, b, a;
	float u, v;

	OmniVert() { Reset(); }

	void Reset()
	{
		x = y = z = 0.0f;
		w = 1.0f;
		r = g = b = 0.0f;
		a = 1.0f;
		u = v = 0.0f;
	}
};


class MeshSection
{
public:
	MeshSection(unsigned int vertex_count, float *data, GLenum primative_type, CTexture *texture, glm::vec2 uvScroll, int pass, PMO_MESH_HEADER* raw);
	~MeshSection();

	void Draw(glm::mat4& model, RenderContext& context, std::shared_ptr<CShader> shader);
	void gui_PrintDetails();
	int PassNumber();
private:
	unsigned int vert_count;
	float *vert_data;

	void setup_ogl();
	GLuint VBO, VAO;
	GLenum draw_primative;
	CTexture *texture;
	bool twoSided;
	uint16 blend;
	glm::vec2 uvScroll;
	int pass;

	PMO_MESH_HEADER* raw;

	friend class AssimpExporter;
};

struct MeshFlags
{
	bool isSkybox;
	//bool isDecal;
};

class Mesh
{
public:
	Mesh();
	~Mesh();

	int mesh_idx;

	void AddSection(unsigned int vertex_count, float *data, GLenum primative_type, CTexture *texture, glm::vec2 uvScroll, int pass, PMO_MESH_HEADER* raw);
	void Draw(RenderContext& context, int pass);
	void SetPosRotScale(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);
	void SetFlags(MeshFlags flags);

	void StoreTextureList(std::vector<std::string> textures);

	unsigned int GetSectionCount();
	bool GetIsSkybox();
	void gui_ListSections();
private:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	std::vector<MeshSection *> sections;
	MeshFlags flags;

	std::vector<std::string> texture_list;	// purly for exporter...

	friend class AssimpExporter;
};

class MeshBuilder
{
public:
	MeshBuilder();

	void BeginSection(GLenum primative_type, CTexture *texture, glm::vec2 uvScroll, int pass);
	void EndSection(PMO_MESH_HEADER* raw);
	
	void TexUV2f(float *uv);
	void Color4f(float *clr);
	void Color3ub(uint8_t *clr);
	void Color4ub(uint8_t *clr);
	void Vertex3f(float *pos);
	void EndVert();

	Mesh *Finish();
private:
	OmniVert current_vert;
	std::vector<OmniVert> vert_list;
	GLenum current_primative;
	CTexture *current_texture;
	glm::vec2 current_uvScroll;
	int current_pass;
	Mesh *child;
};