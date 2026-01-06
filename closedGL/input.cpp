#include "input.h"


bool Input::keys[GLFW_KEY_LAST + 1] = { false };
bool Input::lastFrameKeys[GLFW_KEY_LAST + 1] = { false };


void Input::pollKeyboard(GLFWwindow* window) {
	for (int key = 0; key <= GLFW_KEY_LAST; key++) {
		lastFrameKeys[key] = keys[key];
		keys[key] = (glfwGetKey(window, key) == GLFW_PRESS);
	}
}

bool Input::isKeyPressed(int key) {
	if (key < 0 || key > GLFW_KEY_LAST) {
		return false;
	}

	return keys[key] && !lastFrameKeys[key];

}

bool Input::isKeyReleased(int key) {

	if (key < 0 || key > GLFW_KEY_LAST) {
		return false;
	}

	return !keys[key] && lastFrameKeys[key];

}

bool Input::isKeyDown(int key) {

	if (key < 0 || key > GLFW_KEY_LAST) {
		return false;
	}

	return keys[key];

}
