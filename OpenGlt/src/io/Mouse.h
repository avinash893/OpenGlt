#ifndef MOUSE_H
#define MOUSE_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Mouse
{
public:

	static void cursorposCallback(GLFWwindow* window, double xpos, double ypos);

	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void mouseWheelCallback(GLFWwindow* window, double dx, double dy);


	static double getX();
	static double getY();
	static double DX();
	static double DY();
	static double getScrollDX();
	static double getScrollDY();

	static bool button(int button);
	static bool buttonChanged(int button);
	static bool buttonUp(int button);
	static bool buttonDown(int button);



private:
	static double x;
	static double y;

	static double lastX;
	static double lastY;

	static double dx;
	static double dy;

	static double scrollDX;
	static double scrollDY;

	static bool firstMouse;

	static bool buttons[];
	static bool buttonsChanged[];




};

#endif