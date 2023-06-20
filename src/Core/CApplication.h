#include "GLFW/glfw3.h"
#include "glm/vec2.hpp"
#include <string>

struct Button
{
	bool down, last, last_tick, pressed, pressed_tick;

};

struct Mouse
{
	Button buttons[GLFW_MOUSE_BUTTON_LAST];
	glm::vec2 position, delta;
};

struct Keyboard
{
	Button keys[GLFW_KEY_LAST];
};

class CApplication
{
public:
	CApplication(int argc, char** argv);
	virtual ~CApplication();

	// TODO: Config

	void Run();

protected:
	virtual bool UserPreInit() = 0;
	virtual bool UserInit() = 0;
	virtual bool UserPostInit() = 0;
	virtual void UserPreUpdate() = 0;
	virtual void UserUpdate() = 0;
	virtual void UserDraw() = 0;
	virtual void UserPostPresent() = 0;
	virtual void UserDestroy() = 0;

	std::string windowName;
	GLFWwindow* winHandle;
	glm::ivec2 size;
	Mouse mouse;
	Keyboard keyboard;

	void Present();
private:
	static void Size_Callback(GLFWwindow* handle, int width, int height);
	static void Cursor_Callback(GLFWwindow* handle, double px, double py);
	static void Key_Callback(GLFWwindow* handle, int key, int scancode, int action, int mods);
	static void Button_Callback(GLFWwindow* handle, int button, int action, int mods);

	bool PreInit();
	bool Init();
	bool PostInit();
	void Update();
	void Draw();
	void Destroy();

};