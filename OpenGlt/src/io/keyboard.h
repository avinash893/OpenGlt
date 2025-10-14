#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>


class Keyboard
{
public:
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
		
		
		
		
		/*accesoseries*/


		static bool key(int key);
		static bool keyChanged(int key);
		static bool keyUp(int key);
		static bool keyDown(int key);

	
private:
	static bool  keys[];
	static bool  keysChanged[];



};





#endif // !KEYBOARD_H
