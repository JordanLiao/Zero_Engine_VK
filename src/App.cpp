#include "App.h"
#include "Camera.h"

#include "GLFW/glfw3.h"

App::App() {
    window = nullptr;

    prevTime = std::chrono::steady_clock::now();
    tickTime = 0;
}

App::App(GLFWwindow* w) : App() {
    window = w;
}

void App::tick() {
    std::chrono::steady_clock::time_point currTime = std::chrono::steady_clock::now();
    tickTime = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(currTime - prevTime).count();
    prevTime = currTime;
}

void App::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		keyPressed[key] = true;
	}
	else if (action == GLFW_RELEASE) {
		keyPressed[key] = false;
	}
}

void App::cursor_callback(GLFWwindow* window, double currX, double currY) {
	deltaCursorPos[0] = currX - cursorPos[0];
	deltaCursorPos[1] = currY - cursorPos[1];

	cursorPos[0] = currX;
	cursorPos[1] = currY;
}

void App::mouse_callback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		if(button == GLFW_MOUSE_BUTTON_LEFT) {
			mousePressed[0] = true;
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			mousePressed[1] = true;
		}
		else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
			mousePressed[2] = true;
		}
	}

	if (action == GLFW_RELEASE) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			mousePressed[0] = false;
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			mousePressed[1] = false;
		}
		else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
			mousePressed[2] = false;
		}
	}
}

void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
}
