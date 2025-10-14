#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Button mappings (PlayStation layout)
#define GLFW_JOYSTICK_BTN_LEFT 0         // Square
#define GLFW_JOYSTICK_BTN_DOWN 1         // Cross
#define GLFW_JOYSTICK_BTN_RIGHT 2        // Circle
#define GLFW_JOYSTICK_BTN_UP 3           // Triangle
#define GLFW_JOYSTICK_BTN_SHOULDER_LEFT 4    // L1
#define GLFW_JOYSTICK_BTN_SHOULDER_RIGHT 5   // R1
#define GLFW_JOYSTICK_BTN_TRIGGER_LEFT 6     // L2
#define GLFW_JOYSTICK_BTN_TRIGGER_RIGHT 7    // R2
#define GLFW_JOYSTICK_BTN_SELECT 8       // Share
#define GLFW_JOYSTICK_BTN_START 9        // Options
#define GLFW_JOYSTICK_LEFT_STICK 10      // L3
#define GLFW_JOYSTICK_RIGHT_STICK 11     // R3
#define GLFW_JOYSTICK_HOME 12            // PS
#define GLFW_JOYSTICK_CLICK 13           // Touchpad click
#define GLFW_JOYSTICK_DPAD_UP 14
#define GLFW_JOYSTICK_DPAD_RIGHT 15
#define GLFW_JOYSTICK_DPAD_DOWN 16
#define GLFW_JOYSTICK_DPAD_LEFT 17

// Axis mappings
#define GLFW_JOYSTICK_AXIS_LEFT_X 0
#define GLFW_JOYSTICK_AXIS_LEFT_Y 1
#define GLFW_JOYSTICK_AXIS_RIGHT_X 2
#define GLFW_JOYSTICK_AXIS_LEFT_TRIGGER 3
#define GLFW_JOYSTICK_AXIS_RIGHT_TRIGGER 4
#define GLFW_JOYSTICK_AXIS_RIGHT_Y 5

class Joystick
{
public:
    Joystick(int i);
	void update();

	float axesState(int axis);
	unsigned char buttonState(int button);

    int getAxesCount();
	int getButtonsCount();

	bool isPresent();
	const char* getName();

	static int getId(int i);   


private:
    int present;
    int id;
    const char* name = nullptr;

    int axesCount = 0;
    const float* axes = nullptr;

    int buttonsCount = 0;
    const unsigned char* buttons = nullptr;








};

#endif // JOYSTICK_H
