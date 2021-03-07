#pragma once
#include "Common.h"
#include "Shader.h"
#include "ShaderLibrary.h"

struct RenderContext
{
	float frame_deltaTime;
	double frame_worldTime;

	bool render_wireframe;
	bool render_no_cull;
	bool render_no_blend;
	bool render_no_texture;

	bool render_lighting_enabled;

	glm::mat4 render_viewMatrix;
	glm::mat4 render_projectionMatrix;

	glm::mat4 render_skyViewMatrix;

	float render_nearClip;
	float render_farClip;

	unsigned int stat_objs_drawn;
	unsigned int stat_draw_calls;
	unsigned int stat_tris_drawn;

	unsigned int debug_obj_id;
	unsigned int debug_section_id;
	bool debug_wireframe;
	bool debug_highlight;
	bool debug_no_texture;
	bool debug_active;
	bool debug_section;
	bool debug_peel;
	glm::vec4 debug_highlight_color;

	/*Shader *default_shader;
	Shader *highlight_shader;
	Shader *textureless_shader;*/
	std::string default_shader;
	std::string highlight_shader;
	std::string textureless_shader;

	std::shared_ptr<ShaderLibrary> shaderLibrary;

	bool render_no_fog;
	glm::vec4 fogColor;
	float fogNear;
	float fogFar;
	glm::vec4 clearColor;
};

struct VertexDefinition
{
	size_t vertex_size;			// bytes per vertex
	GLenum primative_type;		// GL_TRIANGLES or GL_TRIANGLE_STRIP
	GLenum position_type;		// GL_NONE, GL_SHORT or GL_FLOAT
	GLenum color_type;			// GL_NONE or GL_UNSIGNED_BYTE or GL_BYTE. Yes, signed. idk either.
	size_t color_components;	// 0, 3 or 4
								// If there are 4 color components the last (alpha) is always unsigned.
	GLenum texture_type;		// GL_NONE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT or GL_FLOAT
};

class IRenderable
{
public:
	virtual ~IRenderable() {};

	virtual bool Create() = 0;
	virtual void Render() = 0;
	virtual void Destroy() = 0;
};