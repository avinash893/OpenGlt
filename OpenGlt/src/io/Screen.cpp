#include "Screen.h"
#include "Mouse.h"
#include "Keyboard.h"

unsigned int Screen::SCR_WIDTH = 800;
unsigned int Screen::SCR_HEIGHT = 600;
void Screen::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	SCR_HEIGHT = height;
	SCR_WIDTH = width;
}

Screen::Screen() :window(nullptr) {};

bool Screen::init()
{
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OGT", nullptr, nullptr);
	if (!window)
	{
		return false;
	}

	glfwMakeContextCurrent(window);
	return true;
}
void Screen::setParameters()
{
	glad_glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, Keyboard::key_callback);
	glfwSetCursorPosCallback(window, Mouse::cursorposCallback);
	glfwSetMouseButtonCallback(window, Mouse::mouseButtonCallback);
	glfwSetScrollCallback(window, Mouse::mouseWheelCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

}

void Screen::update()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Screen::newFrame()
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}

bool Screen::shouldclose()
{
	return glfwWindowShouldClose(window);
}

void Screen::setShouldClose(bool close)
{
	glfwSetWindowShouldClose(window, close);
}