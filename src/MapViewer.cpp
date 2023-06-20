#include "MapViewer.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

MapViewer::MapViewer(int argc, char** argv) : CApplication(argc, argv)
{
	// TODO
}

MapViewer::~MapViewer()
{
	// TODO
}

//virtual bool UserPreInit();
bool MapViewer::UserPreInit()
{
	// TODO: Load config
	// TODO: Create FileManager
	return true;
}

//virtual bool UserInit();
bool MapViewer::UserInit()
{
	if (GLAD_GL_EXT_texture_filter_anisotropic)
	{
		std::cout << "[GS] Loaded extention GL_EXT_texture_filter_anisotropic!" << std::endl;
		float max_anis;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anis);
		std::cout << "[GS] Max anisotropic samples: " << max_anis << std::endl;
	}

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
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigDockingWithShift = true;
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	io.ConfigWindowsResizeFromEdges = true;
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsLight();
	ImGui::GetStyle().ColorButtonPosition = ImGuiDir_Right;
	ImGui::GetStyle().FrameBorderSize = 1.0f;
	ImGui::GetStyle().WindowRounding = 7.0f;
	ImGui_ImplGlfw_InitForOpenGL(winHandle, true);
	ImGui_ImplOpenGL3_Init();

	//std::string fontPath = fileManager.GetFontPath("Roboto-Medium.ttf");
	std::string fontPath = fileManager.GetFontPath("Aldrich-Regular.ttf");
	ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 13.0f);
	assert(font != nullptr);

	return true;
}

bool MapViewer::UserPostInit()
{
	// TODO: SplashScreen
	return true;
}

void MapViewer::UserPreUpdate()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void MapViewer::UserUpdate()
{
	// TODO: Update map
}

void MapViewer::UserDraw()
{
	// TODO: Draw map
	// TODO: Draw GUI

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void MapViewer::UserPostPresent()
{
	// Update and Render additional Platform Windows
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	// TODO: Process ICommand queue
}

void MapViewer::UserDestroy()
{
	// TODO
}