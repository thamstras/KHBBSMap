#include "CMesh.h"

using namespace std::string_literals;

CTexture* CMesh::dummyTexture;
glm::vec2 CMesh::zeroOffset;

CMesh::CMesh()
{
	vertCount = 0;
	polyCount = 0;
	VAO = 0;
	VBO = 0;
	Pos = glm::vec3(0.0f);
	Rot = glm::vec3(0.0f);
	Scale = glm::vec3(1.0f);
}

CMesh::~CMesh()
{
	UnBuild();
}

void CMesh::UnBuild()
{
	if (VAO != 0)
	{
		glDeleteVertexArrays(1, &VAO);
		VAO = 0;
	}
	if (VBO != 0)
	{
		glDeleteBuffers(1, &VBO);
		VBO = 0;
	}

}

void CMesh::Build()
{
	const int attrib_position = 0;
	const int attrib_color = 1;
	const int attrib_tex = 2;

	UnBuild();

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertCount * 10 * sizeof(float), vertData.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(attrib_tex);
	glEnableVertexAttribArray(attrib_color);
	glEnableVertexAttribArray(attrib_position);

	glVertexAttribPointer(attrib_tex,      2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(0 * sizeof(float)));
	glVertexAttribPointer(attrib_color,    4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(2 * sizeof(float)));
	glVertexAttribPointer(attrib_position, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6 * sizeof(float)));

	glBindVertexArray(0);

	if (dummyTexture == nullptr)
	{
		uint8_t whitePixel[] = { 0xFF, 0xFF, 0xFF, 0x80 };
		dummyTexture = new CTexture(1, 1, whitePixel, PF_RGBA32, nullptr);
		zeroOffset = glm::vec2(0.0f);
	}
}

void CMesh::Draw(RenderContext& context)
{
	Draw(context, Pos, Rot, Scale);
}

std::shared_ptr<CShader> CMesh::SelectShader(RenderContext& context)
{
	if (context.debug.highlight)
		return context.render.shaderLibrary->GetShader(context.render.highlight_shader);
	else if (context.render.no_texture)
		return context.render.shaderLibrary->GetShader(context.render.textureless_shader);
	else
		return context.render.shaderLibrary->GetShader(context.render.default_shader);
}

void CMesh::Draw(RenderContext& context, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
{
	auto shader = SelectShader(context);
	shader->use();

	bool flipFace = false;

	glm::quat rot = glm::quat(rotation);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::rotate(model, glm::angle(rot), glm::axis(rot));
	model = glm::scale(model, scale);

	if (glm::determinant(glm::mat3(model)) < 0.0f) flipFace = true;

	glBindVertexArray(VAO);

	// TODO: CShader::set*(...) functions are still fairly expensive despite using std::string literals.
	//		 We should probably cache the uniform locations?
	shader->setMat4("model"s, model);
	if (context.render.currentPass == LAYER_SKY)
		shader->setMat4("view"s, context.render.skyViewMatrix);
	else
		shader->setMat4("view"s, context.render.viewMatrix);	
	shader->setMat4("projection"s, context.render.projectionMatrix);

	for (int i = 0; i < (int)textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		textures[i]->ogl_loadIfNeeded();
		glBindTexture(GL_TEXTURE_2D, textures[i]->getOglId());
	}

	glActiveTexture(GL_TEXTURE0 + textures.size());
	dummyTexture->ogl_loadIfNeeded();
	glBindTexture(GL_TEXTURE_2D, dummyTexture->getOglId());

	if (flipFace) glFrontFace(GL_CW);
	else glFrontFace(GL_CCW);

	unsigned int secBase = 0;
	for (CMeshSection& section : sections)
	{
		if (section.textureIndex != 0xFF)
		{
			shader->setInt("tex_diffuse"s, section.textureIndex);
			shader->setVec2("uv_offset"s, uvOffsets[section.textureIndex]);
		}
		else
		{
			shader->setInt("tex_diffuse"s, textures.size());
			shader->setVec2("uv_offset"s, zeroOffset);
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