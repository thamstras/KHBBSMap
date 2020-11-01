#include "Common.h"
#include "Render.h"
#include "Camera.h"

// NOTES
// Renderer has window passed into it at some point
// Better idea?: Renderer does not init opengl, that way
// we can have multiple renderers.
// but owns it's own camera.
// Come back to this after we've written all the scene stuff so we know what it has to do.
// 

class Renderer
{
public:
	Renderer();
	~Renderer();

	bool Startup();

	void BeginFrame();
	void Update(float deltaTime, double worldTime);
	void Render();
	void EndFrame();

	void Shutdown();

	void Register(IRenderable* object);
	void UnRegister(IRenderable* object);

	void SetupWindowHints();
private:
	Camera camera;

	std::vector<IRenderable*> renderObjects;
	std::vector<IRenderable*> removeList;

	void ResizeFramebuffer(int width, int height);

};

enum RenderPass
{
	PASS_SKYBOX,
	PASS_STATIC,
	PASS_DYNAMIC,
	PASS_UI
};

bool Renderer::Startup()
{
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// ## INIT OPENGL ##

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "[GS] Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return false;
	}

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

	// uncomment this call to draw in wireframe polygons.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	if (USE_ANTIALIASING) glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	// ## INIT IMGUI ##

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(g_window.window, true);
	ImGui_ImplOpenGL3_Init();

	std::string fontPath = fileManager.GetFontPath("Roboto-Medium.ttf");
	ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 13.0f);
	assert(font != nullptr);

	return true;
}

// RENDERER LOOP
// Pump messages
// Process pending "register"s
// Start of Frame
// For each pass
//		Load appropriate matrices into context
//		Call each IRenderable in that passes list
// End of Frame
// Process pending "unregister"s

void Renderer::Render()
{

}
