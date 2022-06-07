#include "CScene.h"
#include <algorithm>

using namespace BBS;

void CScene::Init(FileManager& fileManager)
{
	camera.Reset();
	renderContext.render.current_camera = &camera;
	renderContext.render.wireframe = false;
	renderContext.render.no_blend = false;
	renderContext.render.no_cull = false;
	renderContext.render.no_texture = false;
	
	renderContext.render.nearClip = 0.1f;
	renderContext.render.farClip = 1000.0f;
	
	renderContext.debug.active = false;
	renderContext.debug.sectionOnly = false;
	renderContext.debug.obj_id = 0;
	renderContext.debug.section_id = 0;
	renderContext.debug.wireframe = false;
	renderContext.debug.highlight = false;
	renderContext.debug.no_texture = false;
	renderContext.debug.highlight_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	renderContext.debug.peel = false;

	std::vector<ShaderDef> shaders =
	{
		{
			"unlit_vcol_tex",
			fileManager.GetShaderPath("unlit_vcol_tex.vert.glsl"),
			fileManager.GetShaderPath("unlit_vcol_tex.frag.glsl")
		},
		{
			"constant",
			fileManager.GetShaderPath("constant.vert.glsl"),
			fileManager.GetShaderPath("constant.frag.glsl")
		},
		{
			"unlit_vcol",
			fileManager.GetShaderPath("unlit_vcol.vert.glsl"),
			fileManager.GetShaderPath("unlit_vcol.frag.glsl")
		}
	};

	renderContext.render.shaderLibrary = std::make_shared<ShaderLibrary>(shaders);
	renderContext.render.default_shader = "unlit_vcol_tex";
	renderContext.render.highlight_shader = "constant";
	renderContext.render.textureless_shader = "unlit_vcol";
	
	renderContext.render.no_fog = false;
	renderContext.env.fogColor = glm::vec4(1.0f);
	renderContext.env.fogNear = 1000.0f;
	renderContext.env.fogFar = 1000.0f;
	renderContext.env.clearColor = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f);

	worldContext.deltaTime = 0.0f;
	worldContext.worldTime = 0.0;
	worldContext.frameCount = 0;
	worldContext.render = &renderContext;
}

void CScene::Tick(float deltaTime, double worldTime)
{
	worldContext.deltaTime = deltaTime;
	worldContext.worldTime = worldTime;
	StartFrame();

	theMap->Update(worldContext);

	worldContext.frameCount++;
}

void CScene::Draw()
{
	// TODO: Cap z depth to far clip. (or maybe 2x far clip)
	// Not sure how much it'll help for most maps (Other than rumble racing) but
	// probably a good idea none the less.
	
	glDisable(GL_DEPTH_TEST);
	renderContext.render.currentPass = LAYER_SKY;
	for (CRenderObject* renderObject : renderContext.render.skyDrawList)
		renderObject->DoDraw(renderContext);
	glEnable(GL_DEPTH_TEST);

	renderContext.render.currentPass = LAYER_STATIC;
	std::sort(std::begin(renderContext.render.staticDrawList), std::end(renderContext.render.staticDrawList),
		[this](CRenderObject* a, CRenderObject* b) {return a->CalcZ(renderContext) < b->CalcZ(renderContext); });
	for (CRenderObject* renderObject : renderContext.render.staticDrawList)
		renderObject->DoDraw(renderContext);

	glDepthMask(GL_FALSE);
	renderContext.render.currentPass = LAYER_DYNAMIC;
	std::sort(std::begin(renderContext.render.dynamicDrawList), std::end(renderContext.render.dynamicDrawList),
		[this](CRenderObject* a, CRenderObject* b) {return a->CalcZ(renderContext) > b->CalcZ(renderContext); });
	for (CRenderObject* renderObject : renderContext.render.dynamicDrawList)
		renderObject->DoDraw(renderContext);
	
	renderContext.render.currentPass = LAYER_OVERLAY;
	for (CRenderObject* renderObject : renderContext.render.overlayDrawList)
		renderObject->DoDraw(renderContext);
	glDepthMask(GL_TRUE);

	glDisable(GL_DEPTH_TEST);
	renderContext.render.currentPass = LAYER_GUI;
	for (CRenderObject* renderObject : renderContext.render.guiDrawList)
		renderObject->DoDraw(renderContext);
	glEnable(GL_DEPTH_TEST);
}

void CScene::ProcessKeyboard(GLFWwindow* window)
{
	// TODO: Might be an idea to accumulate camera inputs and process
	// them in Tick where we have this frame's delta not last frame's.
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, worldContext.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, worldContext.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, worldContext.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, worldContext.deltaTime);

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.MovementMultiplier = 2.0f;
	else
		camera.MovementMultiplier = 1.0f;
}

void CScene::ProcessMouse(float deltaX, float deltaY)
{
	camera.ProcessMouseMovement(deltaX, deltaY);
}

void CScene::ProcessMouseScroll(double amount)
{
	camera.ProcessMouseScroll(amount);
}

void CScene::StartFrame()
{
	if (renderContext.render.no_cull)
		glDisable(GL_CULL_FACE);
	else
		glEnable(GL_CULL_FACE);

	if (renderContext.render.wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	renderContext.render.viewMatrix = camera.GetViewMatrix();
	renderContext.render.projectionMatrix = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, renderContext.render.nearClip, renderContext.render.farClip);
	renderContext.render.skyViewMatrix = glm::mat4(glm::mat3(renderContext.render.viewMatrix));

	renderContext.stats.draw_calls = 0;
	renderContext.stats.obj_drawn = 0;
	renderContext.stats.tris_drawn = 0;

	std::shared_ptr<CShader> highlightShader = renderContext.render.shaderLibrary->GetShader(renderContext.render.highlight_shader);
	highlightShader->use();
	highlightShader->setVec4("color", renderContext.debug.highlight_color);

	std::shared_ptr<CShader> standardShader = renderContext.render.shaderLibrary->GetShader(renderContext.render.default_shader);
	standardShader->use();
	standardShader->setVec4("fog_color", renderContext.env.fogColor);
	if (renderContext.render.no_fog)
	{
		standardShader->setFloat("fog_near", renderContext.render.farClip);
		standardShader->setFloat("fog_far", renderContext.render.farClip);
	}
	else
	{
		standardShader->setFloat("fog_near", renderContext.env.fogNear);
		standardShader->setFloat("fog_far", renderContext.env.fogFar);
	}

	glClearColor(renderContext.env.clearColor.r, renderContext.env.clearColor.g, renderContext.env.clearColor.b, renderContext.env.clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderContext.render.skyDrawList.clear();
	renderContext.render.staticDrawList.clear();
	renderContext.render.dynamicDrawList.clear();
	renderContext.render.overlayDrawList.clear();
	renderContext.render.guiDrawList.clear();
}