#include "CApplication.h"
#include "glm/common.hpp"
#include <iostream>
#include "glad/glad.h"

void CApplication::Size_Callback(GLFWwindow* handle, int width, int height)
{
    void* usrPtr = glfwGetWindowUserPointer(handle);
    if (usrPtr != nullptr)
    {
        CApplication* pThis = reinterpret_cast<CApplication*>(usrPtr);
        // TODO: This'll probably interfere with framebuffers!
        glViewport(0, 0, width, height);
        pThis->size.x = width;
        pThis->size.y = height;
    }
}

void CApplication::Cursor_Callback(GLFWwindow* handle, double px, double py)
{
    void* usrPtr = glfwGetWindowUserPointer(handle);
    if (usrPtr != nullptr)
    {
        CApplication* pThis = reinterpret_cast<CApplication*>(usrPtr);
        glm::vec2 p = glm::vec2(px, py);
        // TODO: Raw mouse motion
        // TODO: This subtract may be the wrong way round
        pThis->mouse.delta = p - pThis->mouse.position;
        pThis->mouse.delta = glm::clamp(pThis->mouse.delta, glm::vec2(-100.0f), glm::vec2(100.0f));
        pThis->mouse.position = p;
    }
}

void CApplication::Key_Callback(GLFWwindow* handle, int key, int scancode, int action, int mods)
{
    if (key < 0)
        return;

    void* usrPtr = glfwGetWindowUserPointer(handle);
    if (usrPtr != nullptr)
    {
        CApplication* pThis = reinterpret_cast<CApplication*>(usrPtr);
        switch (action)
        {
        case GLFW_PRESS:
            pThis->keyboard.keys[key].down = true;
            break;
        case GLFW_RELEASE:
            pThis->keyboard.keys[key].down = false;
            break;
        }
    }
}

void CApplication::Button_Callback(GLFWwindow* handle, int button, int action, int mods)
{
    if (button < 0) {
        return;
    }

    void* usrPtr = glfwGetWindowUserPointer(handle);
    if (usrPtr != nullptr)
    {
        CApplication* pThis = reinterpret_cast<CApplication*>(usrPtr);
        switch (action)
        {
        case GLFW_PRESS:
            pThis->mouse.buttons[button].down = true;
            break;
        case GLFW_RELEASE:
            pThis->mouse.buttons[button].down = false;
            break;
        }
    }
}

CApplication::CApplication(int argc, char** argv)
{
    // TODO: something
}

CApplication::~CApplication()
{
    // TODO: Something else
}

bool CApplication::PreInit()
{
	// TODO: Load/Parse config
    windowName = std::string("Window");
    return UserPreInit();
}

bool CApplication::Init()
{
    if (!glfwInit())
    {
        std::cerr << "Error Initialising GLFW!" << std::endl;
        return false;
    }
    
    glfwWindowHint(GLFW_RESIZABLE, true);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    size = glm::vec2(1280, 720);
    winHandle = glfwCreateWindow(size.x, size.y, windowName.c_str(), NULL, NULL);
    if (winHandle == nullptr)
    {
        std::cerr << "Error creating window!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(winHandle);
    glfwSetWindowUserPointer(winHandle, this);

    glfwSetFramebufferSizeCallback(winHandle, &CApplication::Size_Callback);
    glfwSetCursorPosCallback(winHandle, &CApplication::Cursor_Callback);
    glfwSetKeyCallback(winHandle, &CApplication::Key_Callback);
    glfwSetMouseButtonCallback(winHandle, &CApplication::Button_Callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Error initializing GLAD" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwSwapInterval(1);

    return UserInit();
}

bool CApplication::PostInit()
{
	return UserPostInit();
}

void CApplication::Update()
{
	UserUpdate();
}

void CApplication::Draw()
{
	UserDraw();
}

void CApplication::Present()
{
    glfwSwapBuffers(winHandle);
    glfwPollEvents();
}

void CApplication::Destroy()
{
	// NOTE: Unlike other events UserDestroy called *first*
    UserDestroy();
}

void CApplication::Run()
{
	if (!PreInit()) return;
	if (!Init()) return;
	if (!PostInit()) return;

	while (!glfwWindowShouldClose(winHandle))
	{
        UserPreUpdate();
        Update();
		Draw();
        Present();
        UserPostPresent();
	}

	Destroy();
}