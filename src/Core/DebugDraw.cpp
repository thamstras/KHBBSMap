#include "DebugDraw.h"
#include "ShaderLibrary.h"

using namespace std::string_literals;

// Just stashing this here because CoreRender.h doesn't have a corresponding cpp file.
CRenderObject::~CRenderObject() {};

CDebugObject::CDebugObject(float life) : lifetimeRemaining(life)
{

}

CDebugObject::~CDebugObject()
{

}

float CDebugObject::updateLifetime(float deltaTime)
{
	lifetimeRemaining -= deltaTime;
	return lifetimeRemaining;
}

class CDebugCube : public CDebugObject
{
protected:
	static GLuint VBO;
	static GLuint VAO;
	friend class DebugDraw;	// for Teardown()
public:
	CDebugCube(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, glm::vec3 col, float life);
	virtual ~CDebugCube();

	glm::vec3 pos, rot, scale, col;

	virtual void DoDraw(RenderContext& context) override;
	virtual float CalcZ(RenderContext& context) override;
};

std::vector<CDebugObject*> DebugDraw::activeDebugObjects;
GLuint CDebugCube::VBO = 0;
GLuint CDebugCube::VAO = 0;

void DebugDraw::DebugCube(RenderContext& context, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec3 color)
{
	CDebugCube* cube = new CDebugCube(position, rotation, scale, color, 0.0f);
	context.render.dynamicDrawList.push_back(cube);
	DebugDraw::AddDebugObject(cube);
}

float debug_cube_verts[] =
{
	1.0f, 1.0f, 1.0f,	1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, 1.0f,	1.0f, -1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,	-1.0f, 1.0f, 1.0f,

	-1.0f, -1.0f, -1.0f,	-1.0f, -1.0f, 1.0f,
	-1.0f, -1.0f, -1.0f,	-1.0f, 1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,	1.0f, -1.0f, -1.0f,

	-1.0f, 1.0f, 1.0f,	-1.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, 1.0f,	-1.0f, -1.0f, 1.0f,

	1.0f, -1.0f, 1.0f,	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, 1.0f,	-1.0f, -1.0f, 1.0f,

	1.0f, 1.0f, -1.0f,	1.0f, -1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,	-1.0f, 1.0f, -1.0f,
};

CDebugCube::CDebugCube(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, glm::vec3 col, float life)
	: CDebugObject(life), rot(rot), scale(scale), col(col), pos(pos)
{

}

CDebugCube::~CDebugCube()
{

}

void CDebugCube::DoDraw(RenderContext& context)
{
	if (VAO == 0)
	{
		// Create buffers from static data
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, 12 * 3 * sizeof(float), debug_cube_verts, GL_STATIC_DRAW);
		
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		
		glBindVertexArray(0);
	}

	// Draw cube at stored transform using static vao.
	// Suggest using GL_LINES so cube is always wireframe

	glm::quat rot = glm::quat(this->rot);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, this->pos);
	model = glm::rotate(model, glm::angle(rot), glm::axis(rot));
	model = glm::scale(model, this->scale / 2.0f);	// Cube verts define a 'radius' 2 cube, so 1/2 the scale.
	
	std::shared_ptr<CShader> shader = context.render.shaderLibrary->GetShader("debugShader"s);
	shader->setMat4("model"s, model);
	shader->setMat4("view"s, context.render.viewMatrix);
	shader->setMat4("projection"s, context.render.projectionMatrix);
	shader->setVec3("color"s, this->col);
	
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, 24);

	context.stats.draw_calls++;
	context.stats.obj_drawn++;
	context.stats.tris_drawn += 24;	// 12 lines, 2 tris per line
}

float CDebugCube::CalcZ(RenderContext& context)
{
	glm::vec4 objCamPos = context.render.viewMatrix * glm::translate(glm::mat4(1.0f), this->pos) * glm::vec4(this->pos, 1.0f);
	return -objCamPos.z;
}

void DebugDraw::AddDebugObject(CDebugObject* obj)
{
	// try find an empty slot first
	for (auto& itr = activeDebugObjects.begin(); itr != activeDebugObjects.end(); ++itr)
	{
		if (*itr == nullptr)
		{
			*itr = obj;
			return;
		}
	}
	// didn't find an empty slot, add one
	activeDebugObjects.push_back(obj);
}

void DebugDraw::Update(float deltaTime, double worldTime)
{
	for (int i = 0; i < activeDebugObjects.size(); i++)
	{
		CDebugObject* obj = activeDebugObjects[i];
		if (obj->updateLifetime(deltaTime) < 0.0f)
		{
			delete obj;
			activeDebugObjects[i] = nullptr;
		}
	}
}

/*
	Reaches into all the debug object classes (which DebugDraw is friends with)
	and deletes all the buffers.
*/
void DebugDraw::Teardown()
{
	for (auto& obj : activeDebugObjects)
		delete obj;
	activeDebugObjects.clear();

	glDeleteVertexArrays(1, &CDebugCube::VAO);
	glDeleteBuffers(1, &CDebugCube::VBO);
	CDebugCube::VAO = 0;
	CDebugCube::VBO = 0;
}