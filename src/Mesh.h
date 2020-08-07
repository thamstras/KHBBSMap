#pragma once
#include "Common.h"
#include "Shader.h"
#include <cstdint>
#include "Texture.h"
#include "Render.h"

enum VertexFormat
{
	VF_INVALID,
	VF_L3F,				// Location (3 floats)
	VF_L3F_C4F,			// Location (3 floats)	Colour (4 floats)			
	VF_L3F_C3UB,		// Location (3 floats)	Colour (3 UBytes)
	VF_L3F_T2F,			// Location (3 floats)	Texture (2 floats)
	VF_L3F_C4F_T2F,		// Location (3 floats)	Colour (4 floats)	Texture (2 floats)
	VF_L3F_C3UB_T2F,	// Location (3 floats)	Colour (3 UBytes)	Texture (2 floats)
};

enum VFLocationSpecifier
{
	LOC_NONE,
	LOC_3FLOAT,
};

enum VFColourSpecifier
{
	COL_NONE,
	COL_4FLOAT,
	COL_3UBYTE,
};

enum VFTextureSpecifier
{
	TEX_NONE,
	TEX_2FLOAT,
};

VertexFormat SelectFormat(VFLocationSpecifier location, VFColourSpecifier colour, VFTextureSpecifier texture);

struct OmniVert
{
	float x, y, z, w;
	float r, g, b, a;
	float u, v;

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
	MeshSection(unsigned int vertex_count, float *data, GLenum primative_type, Texture *texture);
	~MeshSection();

	void Draw(glm::mat4& model, RenderContext& context, Shader *shader);
	void gui_PrintDetails();
private:
	unsigned int vert_count;
	float *vert_data;

	void setup_ogl();
	GLuint VBO, VAO;
	GLenum draw_primative;
	Texture *texture;
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

	void AddSection(unsigned int vertex_count, float *data, GLenum primative_type, Texture *texture);
	void Draw(RenderContext& context);
	void SetPosRotScale(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);
	void SetFlags(MeshFlags flags);

	unsigned int GetSectionCount();
	bool GetIsSkybox();
	void gui_ListSections();
private:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	std::vector<MeshSection *> sections;
	MeshFlags flags;
};

class MeshBuilder
{
public:
	MeshBuilder();

	void BeginSection(GLenum primative_type, Texture *texture);
	void EndSection();
	
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
	Texture *current_texture;
	Mesh *child;
};