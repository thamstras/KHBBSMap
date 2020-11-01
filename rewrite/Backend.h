#pragma once
#include "GLFW/glfw3.h"
#include "Camera.h"

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
	GLFWwindow* window;
	int width;
	int height;
	bool hasCursorLock;
};

// #### GLOBALS ####
WindowData g_window;
MouseData g_mouse;
Camera g_camera;
double g_worldTime;