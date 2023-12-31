#include "CPlaneSet.h"
#include "Colors.h"

using namespace std::string_literals;

void CPlaneSet::Draw(RenderContext& context, int selectionIdx)
{
	PrepareDraw(context);
	if (currIBO != IBO)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		currIBO = IBO;
	}

	fillShader->use();
	auto lastPlane = planes.size() - 1;
	DrawPlanes(0, lastPlane);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	edgeShader->use();
	edgeShader->setVec4("color"s, Colors::Grey25);
	if (selectionIdx < 0)
	{
		DrawPlanes(0, lastPlane);
	}
	else
	{
		if (selectionIdx > 0)
			DrawPlanes(0, selectionIdx - 1);

		if (selectionIdx < lastPlane)
			DrawPlanes(selectionIdx + 1, lastPlane);
				
		edgeShader->setVec4("color"s, Colors::Red);
		DrawPlanes(selectionIdx, selectionIdx);
	}

	edgeShader->setVec4("color"s, context.debug.highlight_color);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void CPlaneSet::DrawFiltered(RenderContext& context, uint32_t filter, int selectionIdx)
{
	if (filter == 0)
		return;

	PrepareDraw(context);
	PrepareIBO(filter);

	if (minPlane == -1 || maxPlane == -1)
		return;

	fillShader->use();
	DrawPlanes(minPlane, maxPlane, true);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	edgeShader->use();
	edgeShader->setVec4("color"s, Colors::Grey25);
	DrawPlanes(minPlane, maxPlane, true);
	edgeShader->setVec4("color"s, context.debug.highlight_color);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void CPlaneSet::PrepareIBO(uint32_t filter)
{
	if (filter == lastFilter && filteredIBO != 0)
	{
		if (currIBO != filteredIBO)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, filteredIBO);
			currIBO = filteredIBO;
		}
		return;
	}

	std::vector<uint16_t> newIBO = std::vector<uint16_t>();
	minPlane = -1;
	maxPlane = -1;
	for (int i = 0; i < planes.size(); i++)
	{
		auto& plane = planes[i];
		if ((plane.filterData & filter) == 0)
		{
			plane.startIdx = UINT16_MAX;
			continue;
		}

		plane.startIdx = newIBO.size();
		
		newIBO.push_back(plane.baseVertIdx + 0);
		newIBO.push_back(plane.baseVertIdx + 1);
		newIBO.push_back(plane.baseVertIdx + 2);
		if (!plane.isTri)
		{
			newIBO.push_back(plane.baseVertIdx + 0);
			newIBO.push_back(plane.baseVertIdx + 2);
			newIBO.push_back(plane.baseVertIdx + 3);
		}
		if (minPlane == -1) minPlane = i;
		maxPlane = i;
	}

	if (filteredIBO == 0)
		glGenBuffers(1, &filteredIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, filteredIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, newIBO.size() * sizeof(uint16_t), newIBO.data(), GL_DYNAMIC_DRAW);
	currIBO = filteredIBO;
	lastFilter = filter;
}

void CPlaneSet::PrepareDraw(RenderContext& context)
{
	glBindVertexArray(VAO);
	
	auto shader = context.render.shaderLibrary->GetShader("unlit_vcol_alpha");
	shader->use();
	shader->setMat4("model"s, glm::mat4(1.0f));
	shader->setMat4("view"s, context.render.viewMatrix);
	shader->setMat4("projection"s, context.render.projectionMatrix);
	shader->setFloat("alpha"s, alpha);
	fillShader = shader;

	shader = context.render.shaderLibrary->GetShader(context.render.highlight_shader);
	shader->use();
	shader->setMat4("model"s, glm::mat4(1.0f));
	shader->setMat4("view"s, context.render.viewMatrix);
	shader->setMat4("projection"s, context.render.projectionMatrix);
	edgeShader = shader;
	
}

void CPlaneSet::DrawPlanes(int first, int last, bool filtered)
{
	int start, end;
	start =  filtered ? planes[first].startIdx : planes[first].origStartIdx;
	end = filtered ? planes[last].startIdx : planes[last].origStartIdx;
	if (start == UINT16_MAX || end == UINT16_MAX)
		throw std::exception("FUCKUP");
	end += (planes[last].isTri ? 3 : 6);
	glDrawElements(GL_TRIANGLES, end - start, GL_UNSIGNED_SHORT, (void*)(start * sizeof(uint16_t)));
}

void CPlaneSet::Build()
{
	UnBuild();

	std::vector<float> verts = std::vector<float>();
	verts.reserve(planes.size() * 32);	// each plane has 4 verts with 4 pos + 4 color attribs

	std::vector<uint16_t> indicies = std::vector<uint16_t>();
	indicies.reserve(planes.size() * 6);	// each plane is 2 tris
	uint16_t baseIdx = 0;

	for (auto& plane : planes)
	{
		int maxV = plane.isTri ? 3 : 4;
		for (int v = 0; v < maxV; v++)
		{
			verts.push_back(plane.verts[v].x);
			verts.push_back(plane.verts[v].y);
			verts.push_back(plane.verts[v].z);
			verts.push_back(plane.verts[v].w);
			verts.push_back(plane.colors[v].r);
			verts.push_back(plane.colors[v].g);
			verts.push_back(plane.colors[v].b);
			verts.push_back(plane.colors[v].a);
		}

		plane.startIdx = plane.origStartIdx = indicies.size();
		plane.baseVertIdx = baseIdx;
		if (plane.isTri)
		{
			indicies.push_back(baseIdx + 0);
			indicies.push_back(baseIdx + 1);
			indicies.push_back(baseIdx + 2);
			baseIdx += 3;
		}
		else
		{
			indicies.push_back(baseIdx + 0);
			indicies.push_back(baseIdx + 1);
			indicies.push_back(baseIdx + 2);
			indicies.push_back(baseIdx + 0);
			indicies.push_back(baseIdx + 2);
			indicies.push_back(baseIdx + 3);
			baseIdx += 4;
		}
	}

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(uint16_t), indicies.data(), GL_STATIC_DRAW);

	// position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 * sizeof(float)));

	// color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));

	glBindVertexArray(0);
}

void CPlaneSet::UnBuild()
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
	if (IBO != 0)
	{
		glDeleteBuffers(1, &IBO);
		IBO = 0;
	}
	if (filteredIBO != 0)
	{
		glDeleteBuffers(1, &filteredIBO);
		IBO = 0;
	}
}

CPlaneSet::CPlaneSet(std::vector<Plane> planes)
	: planes(std::move(planes))
{
	Build();
}

CPlaneSet::~CPlaneSet()
{
	UnBuild();
}