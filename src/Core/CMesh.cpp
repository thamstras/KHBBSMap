#include "CMesh.h"

using namespace std::string_literals;

CMesh::CMesh()
{

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
	UnBuild();

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertCount * 10 * sizeof(float), vertData.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(0 * sizeof(float)));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(4 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(8 * sizeof(float)));

	glBindVertexArray(0);
}

void CMesh::Draw(RenderContext& context)
{
	Draw(context, Pos, Rot, Scale);
}

void CMesh::Draw(RenderContext& context, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
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

	unsigned int secBase = 0;
	for (CMeshSection& section : sections)
	{
		shader->setInt("tex_diffuse"s, section.textureIndex);
		shader->setVec2("uv_offset"s, uvOffsets[section.textureIndex]);

		for each (auto kick in section.kickList)
		{
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