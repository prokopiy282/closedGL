#pragma once
#include <GLFW/glfw3.h>


class Input {
public:

	static bool isKeyPressed(int key);
	static bool isKeyReleased(int key);
	static bool isKeyDown(int key);
	static void pollKeyboard(GLFWwindow* window);

private:

	static bool keys[GLFW_KEY_LAST + 1];
	static bool lastFrameKeys[GLFW_KEY_LAST + 1];

};