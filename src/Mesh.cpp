#include "Mesh.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

using namespace std::string_literals;

MeshBuilder::MeshBuilder()
{
	current_vert.Reset();
	child = new Mesh();
}

void MeshBuilder::BeginSection(GLenum primative_type, CTexture *texture, glm::vec2 uvScroll, int pass)
{
	current_primative = primative_type;
	vert_list.clear();
	current_vert.Reset();
	current_texture = texture;
	current_uvScroll = uvScroll;
	current_pass = pass;
}

void MeshBuilder::EndSection(PMO_MESH_HEADER* raw)
{
	size_t vert_count = vert_list.size();
	std::vector<float> dat;
	for (int v = 0; v < vert_count; v++)
	{
		dat.push_back(vert_list[v].x);
		dat.push_back(vert_list[v].y);
		dat.push_back(vert_list[v].z);
		dat.push_back(vert_list[v].w);
		dat.push_back(vert_list[v].r);
		dat.push_back(vert_list[v].g);
		dat.push_back(vert_list[v].b);
		dat.push_back(vert_list[v].a);
		dat.push_back(vert_list[v].u);
		dat.push_back(vert_list[v].v);
	}

	//MeshSection(vert_count, dat.data(), current_primative);
	child->AddSection(vert_count, dat.data(), current_primative, current_texture, current_uvScroll, current_pass, raw);
}

void MeshBuilder::TexUV2f(float * uv)
{
	current_vert.u = uv[0];
	current_vert.v = uv[1];
}

void MeshBuilder::Color4f(float * clr)
{
	current_vert.r = clr[0];
	current_vert.g = clr[1];
	current_vert.b = clr[2];
	current_vert.a = clr[3];
}

void MeshBuilder::Color3ub(uint8_t * clr)
{
	float col[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	for (int i = 0; i < 3; i++)
	{
		uint8_t c = clr[i];
		float c2 = ((float)c / 255.0f);
		col[i] = c2;
	}
	Color4f(col);
}

void MeshBuilder::Color4ub(uint8_t * clr)
{
	float col[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	for (int i = 0; i < 4; i++)
	{
		uint8_t c = clr[i];
		float c2 = ((float)c / 255.0f);
		col[i] = c2;
	}
	Color4f(col);
}

void MeshBuilder::Vertex3f(float * pos)
{
	current_vert.x = pos[0];
	current_vert.y = pos[1];
	current_vert.z = pos[2];
}

void MeshBuilder::EndVert()
{
	vert_list.push_back(current_vert);
	current_vert.Reset();
}

Mesh *MeshBuilder::Finish()
{


	return child;
}

MeshSection::MeshSection(unsigned int vertex_count, float *data, GLenum primative_type, CTexture *texture, glm::vec2 uvScroll, int pass, PMO_MESH_HEADER* raw)
{
	this->vert_count = vertex_count;
	//vert_data = (float *)malloc(vertex_count * 10 * sizeof(float));
	vert_data = new float[vertex_count * 10];
	memcpy(vert_data, data, vertex_count * 10 * sizeof(float));
	this->draw_primative = primative_type;
	this->texture = texture;
	this->uvScroll = uvScroll;
	this->raw = raw;
	this->twoSided = raw->attributes & ATTR_BACK;
	this->blend = raw->attributes & ATTR_BLEND_MASK;
	this->pass = pass;

	setup_ogl();
}

MeshSection::~MeshSection()
{
	delete[] vert_data;
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void MeshSection::setup_ogl()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vert_count * 10 * sizeof(float), vert_data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *)(0 * sizeof(float)));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *)(4 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *)(8 * sizeof(float)));

	glBindVertexArray(0);

}

void MeshSection::Draw(glm::mat4& model, RenderContext& context, std::shared_ptr<CShader> shader)
{
	// TODO: Change the Shader interface to use raw c strings.
	// Passing string constants into the Shader interface causes a
	// std::string to be constructed and then immediatly unwrapped all
	// so it can be passed to the ogl api and then destructed again.
	// These 4 lines of code bloat remove the constant construction/destruction
	// of strings, which comes at great cpu cost, but make the code less clear.
	// Since we're almost never going to pass a non-literal to Shader
	// it would probably be best to change it to a raw const char *
	// so that we only pay for the cost of constructing a std::string
	// where we need to.
	//static const std::string uniform_model_name = "model";
	//static const std::string uniform_view_name = "view";
	//static const std::string uniform_projection_name = "projection";

	//shader->use();
	//shader->setMat4(uniform_model_name, model);
	//shader->setMat4(uniform_view_name, context.render_viewMatrix);
	//shader->setMat4(uniform_projection_name, context.render_projectionMatrix);
	
	glm::vec2 uvOffset = glm::vec2(uvScroll.x * context.frame_worldTime * 30.0f, uvScroll.y * context.frame_worldTime * 30.0f);

	texture->ogl_loadIfNeeded();
	GLuint texid = texture->getOglId();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texid);
	shader->setInt("tex_diffuse"s, 0);
	shader->setVec2("uv_offset"s, uvOffset);
	if (twoSided) glDisable(GL_CULL_FACE);
	if (blend != 0 && !context.render_no_blend) glEnable(GL_BLEND);

	glBindVertexArray(VAO);
	glDrawArrays(draw_primative, 0, vert_count);

	if (twoSided && !context.render_no_cull) glEnable(GL_CULL_FACE);
	if (blend != 0 && !context.render_no_blend) glDisable(GL_BLEND);

	context.stat_draw_calls += 1;
	if (draw_primative == GL_TRIANGLES)
		context.stat_tris_drawn += vert_count / 3;
	else if (draw_primative == GL_TRIANGLE_STRIP)
		context.stat_tris_drawn += vert_count - 2;
}

int MeshSection::PassNumber() { return this->pass; }

void MeshSection::gui_PrintDetails()
{
	ImGui::BeginGroup();
	//ImGui::Text("Primative: %s", draw_primative == GL_TRIANGLES ? "TRIANGLES" : "TRIANGLE_STRIP");
	//ImGui::SameLine();
	//ImGui::Text("Vertex Count: %d", vert_count);
	ImGui::Text("Vertex Count: %d", raw->vertexCount);
	ImGui::Text("Vertex size: %d", raw->vertexSize);
	if (ImGui::TreeNode("flags", "Flags: 0x%08X", raw->dataFlags))
	{
		ImGui::Text("Type: %d", raw->dataFlags.primative);
		ImGui::Text("Unknown: %01X", raw->dataFlags.unk);
		ImGui::Text("Diffuse: %d", raw->dataFlags.diffuse);
		ImGui::Text("Skip Transform: %d", raw->dataFlags.skipTransform);
		ImGui::Text("Morph Weights: %d", raw->dataFlags.morphing);
		ImGui::Text("Skinning Weights: %d", raw->dataFlags.skinning);
		ImGui::Text("Index Format: %d", raw->dataFlags.index);
		ImGui::Text("Weights Format: %d", raw->dataFlags.weights);
		ImGui::Text("Position Format: %d", raw->dataFlags.position);
		ImGui::Text("Normal Format: %d", raw->dataFlags.normal);
		ImGui::Text("Color Format: %d", raw->dataFlags.color);
		ImGui::Text("TexCoord: %d", raw->dataFlags.texCoord);
		ImGui::TreePop();
	}
	ImGui::Text("Grp: 0x%02X", raw->group);
	if (ImGui::TreeNode((void*)1, "Attributes: 0x%04X", raw->attributes))
	{
#define FLAGCHECK(X) if (raw->attributes & X) ImGui::Text(#X)
		FLAGCHECK(ATTR_BLEND_NONE);
		FLAGCHECK(ATTR_NOMATERIAL);
		FLAGCHECK(ATTR_GLARE);
		FLAGCHECK(ATTR_BACK);
		FLAGCHECK(ATTR_DIVIDE);
		FLAGCHECK(ATTR_TEXALPHA);
		//FLAGCHECK(FLAG_SHIFT);
		//FLAGCHECK(PRIM_SHIFT);
		FLAGCHECK(ATTR_BLEND_SEMITRANS);
		FLAGCHECK(ATTR_BLEND_ADD);
		FLAGCHECK(ATTR_BLEND_SUB);
		FLAGCHECK(ATTR_BLEND_MASK);
		FLAGCHECK(ATTR_8);
		FLAGCHECK(ATTR_9);
		FLAGCHECK(ATTR_DROPSHADOW);
		FLAGCHECK(ATTR_ENVMAP);
		//FLAGCHECK(ATTR_12);
		//FLAGCHECK(ATTR_13);
		//FLAGCHECK(ATTR_14);
		//FLAGCHECK(ATTR_15);
		//FLAGCHECK(FLAG_COLOR);
		//FLAGCHECK(FLAG_NOWEIGHT);
#undef FLAGCHECK
		ImGui::TreePop();
	}
	ImGui::Separator();
	ImGui::EndGroup();
}

Mesh::Mesh()
{

}

Mesh::~Mesh()
{
	for (auto section : sections)
		delete section;
	sections.clear();
}

void Mesh::AddSection(unsigned int vertex_count, float *data, GLenum primative_type, CTexture *texture, glm::vec2 uvScroll, int pass, PMO_MESH_HEADER* raw)
{
	MeshSection *section = new MeshSection(vertex_count, data, primative_type, texture, uvScroll, pass, raw);
	sections.push_back(section);
}

void Mesh::Draw(RenderContext& context, int pass)
{
	if (flags.isSkybox && pass != 0) return;
	
	bool flipFace = false;

	glm::quat rot = glm::quat(rotation);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::rotate(model, glm::angle(rot), glm::axis(rot));
	model = glm::scale(model, scale);

	if (glm::determinant(glm::mat3(model)) < 0.0f) flipFace = true;

	std::shared_ptr<CShader> shader;
	std::shared_ptr<CShader> sectionShader;

	
	if (context.debug_active && context.debug_obj_id == mesh_idx)
	{
		if (context.debug_section)
		{
			if (context.debug_highlight)
				sectionShader = context.shaderLibrary->GetShader(context.highlight_shader);
			else if (context.debug_no_texture)
				sectionShader = context.shaderLibrary->GetShader(context.textureless_shader);
			else
				sectionShader = context.shaderLibrary->GetShader(context.default_shader);

			// preload sectionShader uniforms
			sectionShader->use();
			sectionShader->setMat4("model"s, model);
			if (flags.isSkybox)
				sectionShader->setMat4("view"s, context.render_skyViewMatrix);
			else
				sectionShader->setMat4("view"s, context.render_viewMatrix);
			sectionShader->setMat4("projection"s, context.render_projectionMatrix);

			shader = context.shaderLibrary->GetShader(context.default_shader);
		}
		else
		{
			if (context.debug_highlight)
				shader = context.shaderLibrary->GetShader(context.highlight_shader);
			else if (context.debug_no_texture)
				shader = context.shaderLibrary->GetShader(context.textureless_shader);
			else
				shader = context.shaderLibrary->GetShader(context.default_shader);

			if (context.debug_wireframe)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			
		}
			
	}
	else if (context.render_no_texture)
	{
		shader = context.shaderLibrary->GetShader(context.textureless_shader);
	}
	else
	{
		shader = context.shaderLibrary->GetShader(context.default_shader);
	}

	shader->use();
	shader->setMat4("model"s, model);
	if (flags.isSkybox)
		shader->setMat4("view"s, context.render_skyViewMatrix);
	else
		shader->setMat4("view"s, context.render_viewMatrix);
	shader->setMat4("projection"s, context.render_projectionMatrix);

	if (flags.isSkybox /*| flags.isDecal*/)
		glDepthMask(GL_FALSE);
	
	if (flipFace) glFrontFace(GL_CW);

	//for (auto& section : sections)
	for (int i = 0; i < sections.size(); i++)
	{
		MeshSection *section = sections[i];
		if (section->PassNumber() != pass && !flags.isSkybox) continue;

		std::shared_ptr<CShader> useShader = shader;

		bool useSectionShader = context.debug_active && context.debug_section
			&& context.debug_obj_id == mesh_idx && context.debug_section_id == i;

		if (useSectionShader)
		{
			useShader = sectionShader;
			sectionShader->use();

			if (context.debug_wireframe)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else if (context.render_wireframe)
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		}

		section->Draw(model, context, useShader);

		if (useSectionShader)
		{
			shader->use();

			if (context.debug_wireframe)
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			else if (context.render_wireframe)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
	}
	context.stat_objs_drawn += 1;

	if (context.debug_active && !context.debug_section
		&& context.debug_obj_id == mesh_idx)
	{
		if (context.debug_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else if (context.render_wireframe || context.debug_peel)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (flags.isSkybox /*| flags.isDecal*/)
		glDepthMask(GL_TRUE);

	if (flipFace) glFrontFace(GL_CCW);
}

void Mesh::SetPosRotScale(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
{
	this->position = pos;
	this->rotation = rot;
	this->scale = scale;
}

void Mesh::SetFlags(MeshFlags flags)
{
	this->flags = flags;
}

void Mesh::StoreTextureList(std::vector<std::string> textures)
{
	this->texture_list = textures;
}

unsigned int Mesh::GetSectionCount()
{
	return sections.size();
}

bool Mesh::GetIsSkybox()
{
	return flags.isSkybox;
}

void Mesh::gui_ListSections()
{
	/*for (auto s : sections)
	{
		s->gui_PrintDetails();
	}*/
	for (int s = 0; s < sections.size(); s++)
	{
		if (ImGui::TreeNode((void*)(intptr_t)s, "Section %d", s))
		{
			sections[s]->gui_PrintDetails();
			ImGui::TreePop();
		}
	}
}