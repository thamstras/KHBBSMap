#pragma once
#include "..\Common.h"
#include "CoreRender.h"

/*

A somewhat strange thing. Not 100% sure I like this being in Core, but nevermind.

*/

class CPlaneSet
{
public:
	struct Plane
	{
	public:
		// The 3/4 verts of the plane
		glm::vec4 verts[4];
		// The 3/4 colors of the plane
		glm::vec4 colors[4];
		// filter data
		uint32_t filterData;
		// Where this plane begins in the current indices array
		uint16_t startIdx;
		// Where this plane begins in the full indices array
		uint16_t origStartIdx;
		// Where this plane begins in the vertex array
		uint16_t baseVertIdx;
		bool isTri;
	};

	float alpha = 0.5f;

	CPlaneSet(std::vector<Plane> planes);
	~CPlaneSet();

	void Draw(RenderContext& context, int selectionIdx = -1);
	void DrawFiltered(RenderContext& context, uint32_t filter, int selectionIdx = -1);

private:
	void PrepareIBO(uint32_t filter);
	void PrepareDraw(RenderContext& context);
	void DrawPlanes(int first, int last, bool filtered = false);

	void Build();
	void UnBuild();

	std::vector<Plane> planes;
	GLuint VBO = 0, VAO = 0, IBO = 0;
	GLuint filteredIBO = 0;
	uint32_t lastFilter = 0;
	int minPlane = -1, maxPlane = -1;
	GLuint currIBO;

	std::shared_ptr<CShader> fillShader;
	std::shared_ptr<CShader> edgeShader;
};