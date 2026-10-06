#ifndef Screen_H
#define Screen_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>


class Screen
{
public :
	static unsigned int SCR_WIDTH;
	static unsigned int SCR_HEIGHT;

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

	Screen();

	bool init();
	void setParameters();

	GLFWwindow* getWindow() { return window; }

	void update();
	void newFrame();

	bool shouldclose();
	void setShouldClose(bool close);
	void setCursorMode(int mode);
	int getCursorMode();


private:
	GLFWwindow* window;

};



#endif