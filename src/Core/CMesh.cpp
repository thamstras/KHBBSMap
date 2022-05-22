#include "CMesh.h"

using namespace std::string_literals;

CMesh::CMesh()
{
	vertCount = 0;
	polyCount = 0;
	VAO = 0;
	VBO = 0;
	Pos = glm::vec3(0.0f);
	Rot = glm::vec3(0.0f);
	Scale = glm::vec3(0.0f);
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
}

void CMesh::Draw(RenderContext& context)
{
	Draw(context, Pos, Rot, Scale);
}

void CMesh::Draw(RenderContext& context, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
{
	auto shader = context.render.shaderLibrary->GetShader(context.render.default_shader);
	shader->use();

	bool flipFace = false;

	glm::quat rot = glm::quat(rotation);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::rotate(model, glm::angle(rot), glm::axis(rot));
	model = glm::scale(model, scale);

	if (glm::determinant(glm::mat3(model)) < 0.0f) flipFace = true;

	glBindVertexArray(VAO);

	shader->setMat4("model"s, model);
	if (context.render.currentPass == LAYER_SKY)
		shader->setMat4("view"s, context.render.skyViewMatrix);
	else
		shader->setMat4("view"s, context.render.viewMatrix);	
	shader->setMat4("projection"s, context.render.projectionMatrix);

	for (int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		textures[i]->ogl_loadIfNeeded();
		glBindTexture(GL_TEXTURE_2D, textures[i]->getOglId());
	}

	if (flipFace) glFrontFace(GL_CW);
	else glFrontFace(GL_CCW);

	unsigned int secBase = 0;
	for (CMeshSection& section : sections)
	{
		shader->setInt("tex_diffuse"s, section.textureIndex);
		shader->setVec2("uv_offset"s, uvOffsets[section.textureIndex]);
		if (section.twoSided) glDisable(GL_CULL_FACE);
		else glEnable(GL_CULL_FACE);
		if (section.blend) glEnable(GL_BLEND);
		else glDisable(GL_BLEND);

		for each (auto kick in section.kickList)
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