#include  "keyboard.h"
#include <iostream>

bool Keyboard::keys[GLFW_KEY_LAST] = { 0 };
bool Keyboard::keysChanged[GLFW_KEY_LAST] = { 0 };

void Keyboard::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_RELEASE)
	{
		if (!keys[key])
		{
			keys[key] = true;
			keysChanged[key] = true;
		}
	}
		else
		{
			keys[key] = false;
		}

		keysChanged[key] = action != GLFW_REPEAT;
 }

bool Keyboard::key(int key)
{
	return keys[key];

 }
bool Keyboard::keyChanged(int key)
{
	bool ret = keysChanged[key];
	keysChanged[key] = false;
	return ret; 
 }
bool Keyboard::keyUp(int key)
{
	return !keys[key] && keyChanged(key);
	//return !keys  that is is key is false and is recently chanegd 

 }

bool Keyboard::keyDown(int key)
{
	return keys[key] && keyChanged(key);
	//return keys that is is key is true and is recently chanegd

 }

