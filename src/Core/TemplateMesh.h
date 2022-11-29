#pragma once
#include "Common.h"
#include "CoreRender.h"
#include "VertexTypes.h"
#include "CTexture.h"

struct MeshSection
{
	// TODO: do we actually need vertCount or stride?
	int vertCount;
	int textureIndex;
	unsigned int stride;
	GLenum primType;
	bool twoSided;
	bool blend;
	std::vector<int> kickList;
};

template <class VertexType>
class CMesh
{
	static_assert(std::is_trivial_v<VertexType>);
	static_assert(std::is_standard_layout_v<VertexType>);

	GLuint VAO;
	GLuint VBO;

	std::vector<VertexType> verts;
	std::vector<MeshSection> sections;
	
	std::vector<CTexture*> textures;
	std::vector<glm::vec2> uvOffsets;

public:
	CMesh(std::vector<VertexType> verts, std::vector<MeshSection> sections, std::vector<CTexture*> textures) :
		verts(std::move(verts)), sections(std::move(sections)), textures(std::move(textures)), uvOffsets(textures.size())
	{
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(VertexType), verts.data(), GL_STATIC_DRAW);

		VertexType::BindVAO();

		glBindVertexArray(0);
	}

	~CMesh()
	{
		if (VAO != 0)
			glDeleteVertexArrays(1, &VAO);
		VAO = 0;

		if (VBO != 0)
			glDeleteBuffers(1, &VBO);
		VBO = 0;
	}

	CMesh(const CMesh& other) = delete;
	CMesh& operator=(const CMesh& other) = delete;

	CMesh(CMesh&& other) noexcept
	{
		// TODO;
	}

	CMesh& operator=(CMesh&& other) noexcept
	{
		// TODO;
	}

	std::shared_ptr<CShader> SelectShader(RenderContext& context)
	{
		// TODO: Needs updating to take note of vertex capabilities

		if (context.debug.highlight)
			return context.render.shaderLibrary->GetShader(context.render.highlight_shader);
		else if (context.render.no_texture)
			return context.render.shaderLibrary->GetShader(context.render.textureless_shader);
		else
			return context.render.shaderLibrary->GetShader(context.render.default_shader);
	}

	void Draw(RenderContext& context, const glm::vec3 pos, const glm::vec3 rot, const glm::vec3 scale)
	{
		glm::quat qrot = glm::quat(rot);
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, pos);
		model = glm::rotate(model, glm::angle(qrot), glm::axis(qrot));
		model = glm::scale(model, scale);

		Draw(context, model);
	}

	void Draw(RenderContext& context, const glm::mat4& transform)
	{
		static const glm::vec2 zeroUVOffset = glm::vec2(0.0f);

		using namespace std::string_literals;

		auto shader = SelectShader(context);
		shader->use();

		bool flipFace = false;
		//if (glm::determinant(glm::mat3(transform)) < 0.0f) flipFace = true;
		// Note: This is faster to compute than the above, but only correct in *most* circumstances.
		if (transform[0][0] * transform[1][1] * transform[2][2] < 0.0f) flipFace = true;

		glBindVertexArray(VAO);

		// TODO: CShader::set*(...) functions are still fairly expensive despite using std::string literals.
		//		 We should probably cache the uniform locations?
		shader->setMat4("model"s, transform);
		
		// TODO: These 2 should be set by the RenderPass code when the pass starts
		if (context.render.currentPass == LAYER_SKY)
			shader->setMat4("view"s, context.render.skyViewMatrix);
		else
			shader->setMat4("view"s, context.render.viewMatrix);
		shader->setMat4("projection"s, context.render.projectionMatrix);

		if constexpr (VertexType::supportsTextures)
		{
			// TODO: Test and see if this (slightly odd) way of handling textures is
			// actually faster.
			int i = 0;
			for (CTexture* tex : textures)
			{
				glActiveTexture(GL_TEXTURE0 + i);
				tex->ogl_loadIfNeeded();
				glBindTexture(GL_TEXTURE_2D, tex->getOglId());
				i++;
			}

			// TODO: stash dummy texture in render context
			//glActiveTexture(GL_TEXTURE0 + i);
			//dummyTexture->ogl_loadIfNeeded();
			//glBindTexture(GL_TEXTURE_2D, dummyTexture->getOglId());
		}

		if (flipFace) glFrontFace(GL_CW);
		else glFrontFace(GL_CCW);

		int secBase = 0;
		for (MeshSection& section : sections)
		{
			if constexpr (VertexType::supportsTextures)
			{
				if (section.textureIndex != 0xFF)
				{
					shader->setInt("tex_diffuse"s, section.textureIndex);
					shader->setVec2("uv_offset"s, uvOffsets[section.textureIndex]);
				}
				else
				{
					shader->setInt("tex_diffuse"s, textures.size());
					shader->setVec2("uv_offset"s, zeroUVOffset);
				}
			}

			if (section.twoSided || context.render.no_cull) glDisable(GL_CULL_FACE);
			else glEnable(GL_CULL_FACE);

			if (section.blend && !context.render.no_blend) glEnable(GL_BLEND);
			else glDisable(GL_BLEND);

			for (auto kick : section.kickList)
			{
				if (kick == 0) continue;
				glDrawArrays(section.primType, secBase, kick);
				secBase += kick;
				context.stats.draw_calls++;
				if (section.primType == GL_TRIANGLES)
					context.stats.tris_drawn += kick / 3;
				else if (section.primType == GL_TRIANGLE_STRIP)
					context.stats.tris_drawn += kick - 2;
			}
		}

		context.stats.obj_drawn += 1;
	}
};