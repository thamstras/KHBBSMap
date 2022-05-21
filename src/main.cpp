// #### INCLUDES ####
#include "Common.h"

#include <filesystem>
#include "FileManager.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Core\CoreRender.h"
#include "Core\CShader.h"
#include "Core\ShaderLibrary.h"
#include "Core\CCamera.h"
#include "Core\World.h"

#include "BBS\CScene.h"
#include "BBS\CMap.h"

//#include "MapLoad.h"
//#include "Render.h"
#include "glm/gtc/type_ptr.hpp"

// #### STRUCTS ####
struct MouseData
{
	double xPos;
	double yPos;
	float deltaX;
	float deltaY;
	bool firstData;
	bool rawMotionAvailible;
};

struct WindowData
{
	GLFWwindow *window;
	int width;
	int height;
	bool hasCursorLock;
};

// #### SETTINGS ####
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

const bool USE_ANTIALIASING = true;
const bool USE_ANISOTROPIC = false;

const float MAX_DELTA_TIME = 0.2f;

const bool DISABLE_MOUSELOCK = true;

// #### GLOBALS ####
WindowData g_window;
MouseData g_mouse = { 0.0, 0.0, 0.0f, 0.0f, true, false };
//CCamera g_camera(glm::vec3(0.0f, 1.5f, -3.0f));
BBS::CScene* theScene;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
double g_worldTime = 0.0f;

bool gui_show_metrics = false;

bool g_loadNewMap = false;

// #### CALLBACKS ####
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	g_window.width = width;
	g_window.height = height;
}

void lockCursor()
{
	glfwGetCursorPos(g_window.window, &g_mouse.xPos, &g_mouse.yPos);
	glfwSetInputMode(g_window.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (g_mouse.rawMotionAvailible)
		glfwSetInputMode(g_window.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	g_window.hasCursorLock = true;
}

void unlockCursor()
{
	glfwSetInputMode(g_window.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	if (g_mouse.rawMotionAvailible)
		glfwSetInputMode(g_window.window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
	g_window.hasCursorLock = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (g_mouse.firstData)
	{
		g_mouse.xPos = xpos;
		g_mouse.yPos = ypos;
		g_mouse.deltaX = 0.0f;
		g_mouse.deltaY = 0.0f;
		g_mouse.firstData = false;
	}

	g_mouse.deltaX = xpos - g_mouse.xPos;
	g_mouse.deltaY = g_mouse.yPos - ypos; // reversed since y-coordinates go from bottom to top

	g_mouse.xPos = xpos;
	g_mouse.yPos = ypos;

	if (ImGui::GetIO().WantCaptureMouse)
	{
		if (g_window.hasCursorLock) unlockCursor();
		return;
	}

	if (g_window.hasCursorLock)
		g_camera.ProcessMouseMovement(g_mouse.deltaX, g_mouse.deltaY);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	/*if (g_window.hasCursorLock)*/ 
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		g_camera.ProcessMouseScroll(yoffset);
	}
}

void processInput(GLFWwindow *window)
{
	if (ImGui::GetIO().WantCaptureKeyboard)
	{
		if (g_window.hasCursorLock) unlockCursor();
		return;
	}

	if (!ImGui::GetIO().WantCaptureMouse
		&& glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
	{
		if (!g_window.hasCursorLock) lockCursor();
	}
	else if (DISABLE_MOUSELOCK && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE)
	{
		if (g_window.hasCursorLock) unlockCursor();
	}
	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
	{
		if (g_window.hasCursorLock) unlockCursor();
	}

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		g_camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		g_camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		g_camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		g_camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		g_camera.MovementMultiplier = 2.0f;
	else
		g_camera.MovementMultiplier = 1.0f;
}

// #### FUNCTIONS

bool init(FileManager& fileManager)
{
	// ## INIT GLFW ##

	glfwInit();
	std::cout << "[GS] Compiled with GLFW " << glfwGetVersionString() << std::endl;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	if (USE_ANTIALIASING) glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "KHBBSMap", NULL, NULL);
	g_window.window = window;
	if (window == NULL)
	{
		std::cout << "[GS] Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (glfwRawMouseMotionSupported()) g_mouse.rawMotionAvailible = true;

	// ## INIT OPENGL ##

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "[GS] Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return false;
	}

	//setupGLDebug();

	if (GLAD_GL_EXT_texture_filter_anisotropic)
	{
		std::cout << "[GS] Loaded extention GL_EXT_texture_filter_anisotropic!" << std::endl;
		float max_anis;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anis);
		std::cout << "[GS] Max anisotropic samples: " << max_anis << std::endl;
	}

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	g_window.width = SCR_WIDTH;
	g_window.height = SCR_HEIGHT;

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	std::cout << "[GS] Renderer: " << renderer << std::endl;
	std::cout << "[GS] OpenGL version: " << version << std::endl;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	if (USE_ANTIALIASING) glEnable(GL_MULTISAMPLE);
	glEnable(GL_LINE_SMOOTH);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glDepthFunc(GL_LEQUAL);

	// ## INIT IMGUI ##

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsLight();
	ImGui::GetStyle().ColorButtonPosition = ImGuiDir_Left;
	ImGui::GetStyle().FrameBorderSize = 1.0f;
	ImGui_ImplGlfw_InitForOpenGL(g_window.window, true);
	ImGui_ImplOpenGL3_Init();
	
	//std::string fontPath = fileManager.GetFontPath("Roboto-Medium.ttf");
	std::string fontPath = fileManager.GetFontPath("Aldrich-Regular.ttf");
	ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 13.0f);
	assert(font != nullptr);


	return true;
}

void Render_StartFrame(RenderContext& context)
{
	if (context.render.no_cull)
		glDisable(GL_CULL_FACE);
	else
		glEnable(GL_CULL_FACE);

	if (context.render.wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	context.render.viewMatrix = g_camera.GetViewMatrix();
	context.render.projectionMatrix = glm::perspective(glm::radians(g_camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, context.render.nearClip, context.render.farClip);
	context.render.skyViewMatrix = glm::mat4(glm::mat3(context.render.viewMatrix));

	context.stats.draw_calls = 0;
	context.stats.obj_drawn = 0;
	context.stats.tris_drawn = 0;

	//context.highlight_shader->use();
	//context.highlight_shader->setVec4("color", context.debug_highlight_color);
	std::shared_ptr<CShader> highlightShader = context.render.shaderLibrary->GetShader(context.render.highlight_shader);
	highlightShader->use();
	highlightShader->setVec4("color", context.debug.highlight_color);

	std::shared_ptr<CShader> standardShader = context.render.shaderLibrary->GetShader(context.render.default_shader);
	standardShader->use();
	standardShader->setVec4("fog_color", context.env.fogColor);
	if (context.render.no_fog)
	{
		standardShader->setFloat("fog_near", context.render.farClip);
		standardShader->setFloat("fog_far", context.render.farClip);
	}
	else
	{
		standardShader->setFloat("fog_near", context.env.fogNear);
		standardShader->setFloat("fog_far", context.env.fogFar);
	}

	glClearColor(context.env.clearColor.r, context.env.clearColor.g, context.env.clearColor.b, context.env.clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//bool gui_show_map_data = false;
//bool gui_show_data_viewer = false;
bool gui_show_debug_tool = false;
//bool gui_show_object_viewer = false;

void gui_DrawOpenFileModal()
{
	if (ImGui::BeginPopupModal("OpenFileModal"))
	{
		if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

//bool export_option = false;

//bool export_skybox_normal = true;
//bool export_skybox_scaled = false;

/*void gui_DrawExportOptions(FileManager& filemanager)
{
	if (ImGui::BeginPopupModal("ExportModal"))
	{
		ImGui::Text("Export Options");

		ImGui::Separator();
		
		ImGui::Text("Skybox handling (NYI)");
		bool n_normal = ImGui::RadioButton("Normal", export_skybox_normal);
		ImGui::SameLine();
		bool n_scaled = ImGui::RadioButton("Scaled", export_skybox_scaled);
		if (n_normal) { export_skybox_normal = true; export_skybox_scaled = false; }
		else if (n_scaled) { export_skybox_normal = false; export_skybox_scaled = true; }

		ImGui::Separator();

		ImGui::Text("Will export to .\\resources\\export");

		ImGui::Separator();

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Export"))
		{
			ImGui::CloseCurrentPopup();
			std::string fs;
			if (filemanager.GetExportFolder(fs))
			{
				ExportMap(fs);
			}
		}

		ImGui::EndPopup();
	}
}*/

void gui_DrawSystemGui(FileManager& fileManager)
{
	// 'System' info window
	ImGui::Begin("System");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	//ImGui::Text("Wall time: 0.0s Game time: %.3fs", g_worldTime);

	ImGui::TextColored(ImVec4(0, 0.75f, 0.1125f, 1.0f), "Camera");
	ImGui::Text("Pos { %.2f, %.2f, %.2f } Pitch %.2f Yaw %.2f",
		g_camera.Position.x, g_camera.Position.y, g_camera.Position.z,
		g_camera.Pitch, g_camera.Yaw);
	if (g_window.hasCursorLock)
		ImGui::TextColored(ImVec4(0, 0.75f, 0.1125f, 1.0f), "Focused");
	else
		ImGui::TextColored(ImVec4(1.0f, 0, 0, 1.0f), "Unfocused");

	//ImGui::Text("Our mouse   { %.2f, %.2f }", g_mouse.xPos, g_mouse.yPos);

	//ImGui::Checkbox("Map File", &gui_show_map_data);
	//ImGui::SameLine();
	//ImGui::Checkbox("Data View", &gui_show_data_viewer);
	//ImGui::SameLine();
	ImGui::Checkbox("Debug Tools", &gui_show_debug_tool);

	if (ImGui::Button("Metrics"))
		gui_show_metrics = !gui_show_metrics;
	ImGui::End();

	if (gui_show_metrics)
		ImGui::ShowMetricsWindow();

	// System Gui (toolbar etc.)
	bool shouldOpenFile = false;
	bool shouldCloseFile = false;
	bool shouldExport = false;
	bool shouldExit = false;

	ImGui::BeginMainMenuBar();
	
	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("Open...")) shouldOpenFile = true;
		//if (ImGui::MenuItem("Close")) shouldCloseFile = true;
		ImGui::Separator();
		if (ImGui::MenuItem("Export...")) shouldExport = true;
		ImGui::Separator();
		if (ImGui::MenuItem("Exit")) shouldExit = true;
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();

	//if (shouldOpenFile) ImGui::OpenPopup("OpenFileModal");
	if (shouldExport) ImGui::OpenPopup("ExportModal");

	if (shouldExit) glfwSetWindowShouldClose(g_window.window, true);

	if (shouldOpenFile)
	{
		g_loadNewMap = true;
	}


	// Modals
	//gui_DrawOpenFileModal();
	//gui_DrawExportOptions(fileManager);
}

void gui_DrawRenderGui(RenderContext& context)
{
	ImGui::Begin("Render");
	ImGui::Checkbox("Wireframe", &context.render.wireframe);
	ImGui::Checkbox("Disable Blend", &context.render.no_blend);
	ImGui::Checkbox("Disable Culling", &context.render.no_cull);
	ImGui::Checkbox("Disable Textures", &context.render.no_texture);
	ImGui::Separator();
	ImGui::Text("Objs: %d, Tris: %d", context.stats.obj_drawn, context.stats.tris_drawn);
	ImGui::Text("Draw Calls: %d", context.stats.draw_calls);
	ImGui::End();
}

void gui_DrawDebugGui(RenderContext& context)
{
	ImGui::Begin("Debug");
	ImGui::Checkbox("Enable Debug", &context.debug.active);
	
	ImGui::Separator();
	
	if (!context.debug.active)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
	}

	ImGui::InputInt("object_id", (int *)&context.debug.obj_id);
	ImGui::InputInt("section_id", (int *)&context.debug.section_id);
	ImGui::ColorEdit4("highlight_color", glm::value_ptr(context.debug.highlight_color));
	ImGui::Checkbox("Section only", &context.debug.sectionOnly);
	ImGui::SameLine();
	ImGui::Checkbox("Highlight", &context.debug.highlight);
	ImGui::SameLine();
	ImGui::Checkbox("Textureless", &context.debug.no_texture);
	ImGui::Checkbox("Wireframe", &context.debug.wireframe);
	ImGui::SameLine();
	ImGui::Checkbox("Peel", &context.debug.peel);

	if (!context.debug.active)
	{
		ImGui::PopStyleColor();
	}

	//ImGui::Separator();

	//ImGui::Checkbox("Object Viewer", &gui_show_object_viewer);

	ImGui::End();

	if (context.debug.obj_id > 2 * max_object_id())
		context.debug.obj_id = max_object_id();
	else if (context.debug.obj_id > max_object_id())
		context.debug.obj_id = 0;

	//if (gui_show_object_viewer)
	//	gui_object_view(context.debug.obj_id);

	//context.debug_active = false;
}

void gui_DrawEnvGui(FileManager& fileManager, RenderContext& context)
{
	bool loadPVD = false;
	float viewAngle = glm::radians(g_camera.Zoom);
	ImGui::Begin("Environment");
	
	ImGui::ColorEdit4("Fog Color", glm::value_ptr(context.env.fogColor));
	
	ImGui::SetNextItemWidth(0.4f * ImGui::CalcItemWidth());
	ImGui::InputFloat("Fog Near", &context.env.fogNear);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(0.4f * ImGui::CalcItemWidth());
	ImGui::InputFloat("Fog Far", &context.env.fogFar);
	
	ImGui::SetNextItemWidth(0.4f * ImGui::CalcItemWidth());
	ImGui::InputFloat("Near Clip", &context.render.nearClip);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(0.4f * ImGui::CalcItemWidth());
	ImGui::InputFloat("Far Clip", &context.render.farClip);
	
	ImGui::ColorEdit4("Clear Color", glm::value_ptr(context.env.clearColor));
	
	ImGui::SetNextItemWidth(0.4f * ImGui::CalcItemWidth());
	if (ImGui::InputFloat("View Angle", &viewAngle))
	{
		g_camera.Zoom = glm::degrees(viewAngle);
	}
	
	if (ImGui::Button("Load Env (PVD)"))
	{
		loadPVD = true;
	}

	ImGui::End();

	if (loadPVD)
	{
		std::string path;
		if (fileManager.OpenFileWindow(path))
		{
			// TODO: better.
			std::FILE* file = std::fopen(path.c_str(), "rb");
			char magic[4];
			std::fread(magic, 1, 4, file);
			if (memcmp(magic, "PVD\0", 4) != 0)
			{
				std::cerr << "Not a valid PVD file!" << std::endl;
				return;
			}

			// fog color
			std::fseek(file, 0x10, SEEK_SET);
			glm::u8vec4 colorIn;
			std::fread((void*)(glm::value_ptr(colorIn)), sizeof(glm::u8), 4, file);
			context.env.fogColor = glm::vec4((float)colorIn.r / 255.0f, (float)colorIn.g / 255.0f, (float)colorIn.b / 255.0f, (float)colorIn.a / 255.0f);

			std::fread((void*)(&context.env.fogNear), sizeof(float), 1, file);
			std::fread((void*)(&context.env.fogFar), sizeof(float), 1, file);
			std::fread((void*)(&context.render.nearClip), sizeof(float), 1, file);
			std::fread((void*)(&context.render.farClip), sizeof(float), 1, file);

			// clear color
			std::fseek(file, 0x28, SEEK_SET);
			std::fread((void*)(glm::value_ptr(colorIn)), sizeof(glm::u8), 4, file);
			context.env.clearColor = glm::vec4((float)colorIn.r / 255.0f, (float)colorIn.g / 255.0f, (float)colorIn.b / 255.0f, (float)colorIn.a / 255.0f);

			std::fseek(file, 0x30, SEEK_SET);
			float viewIn;
			std::fread((void*)(&viewIn), sizeof(float), 1, file);
			g_camera.Zoom = glm::degrees(viewIn);

			std::fclose(file);
		}
	}
}

void shutdown_gs()
{
	//UnloadBBSMap();

	std::cout << "[GS] Shutting down..." << std::endl;
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(g_window.window);
	glfwTerminate();

	std::cout << "[GS] Goodbye!" << std::endl;
}

void gui_splash()
{
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// TODO: Draw spash image
	//		Load texture
	//		Display fullscreen

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(g_window.window);
}

void gui_endSplash()
{
	// TODO: delete texture
}

void LoadNewMap(FileManager& fileManager, RenderContext renderContext)
{
	std::string newFile;
	if (fileManager.OpenFileWindow(newFile))
	{
		//UnloadBBSMap();
		//LoadBBSMap(newFile);
		//ParseLoadedMap();
		//LoadMapTextures();
		//LoadMapObjects();

		renderContext.debug.obj_id = 0;
		renderContext.debug.section_id = 0;

		g_camera.Reset(glm::vec3(0.0f, 1.5f, -3.0f));
	}
}

// #### MAIN ####
int main(int argc, char **argv)
{	
	FileManager fileManager = FileManager();
	
	if (!init(fileManager))
		return -1;
	atexit(shutdown_gs);

	lastFrame = glfwGetTime();

	gui_splash();

	std::string loadPath;

	BBS::CScene* scene = new BBS::CScene();
	scene->Init();

	if (!fileManager.OpenFileWindow(loadPath))
	{
		return 0;
	}

	//LoadBBSMap(loadPath);
	//ParseLoadedMap();
	//LoadMapTextures();
	//LoadMapObjects();

	// TODO: CScene::OpenMap(...);
	BBS::CMap *map = new BBS::CMap();
	map->LoadMapFile(loadPath);
	scene->theMap = map;

	gui_endSplash();

	while (!glfwWindowShouldClose(g_window.window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		if (deltaTime > MAX_DELTA_TIME) deltaTime = MAX_DELTA_TIME;
		lastFrame = currentFrame;
		g_worldTime += deltaTime;

		//globalRenderContext.frame_deltaTime = deltaTime;
		//globalRenderContext.frame_worldTime = g_worldTime;

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		processInput(g_window.window);

		//Render_StartFrame(globalRenderContext);
		
		//RenderBBSMap(globalRenderContext);
		scene->Tick(deltaTime, g_worldTime);

		scene->Draw();

		gui_DrawSystemGui(fileManager);

		//if (gui_show_map_data) gui_MapData();
		//if (gui_show_data_viewer) gui_loaded_data();
		if (gui_show_debug_tool) gui_DrawDebugGui(globalRenderContext);
		//gui_tex_view();

		gui_DrawRenderGui(globalRenderContext);
		gui_DrawEnvGui(fileManager, globalRenderContext);

		//ImGui::ShowDemoWindow();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(g_window.window);
		glfwPollEvents();

		if (g_loadNewMap)
		{
			g_loadNewMap = false;
			LoadNewMap(fileManager, globalRenderContext);
		}

	}

	/*ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(g_window.window);
	glfwTerminate();*/

	return 0;
}