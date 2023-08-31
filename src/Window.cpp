#include "Window.h"

#include <iostream>
#include <stdexcept>


Window::Window(int width, int height, const char* windowName) {
	this->width = width;
	this->height = height;
	this->windowName = windowName;

	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialize GLFW");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(width, height, windowName, nullptr, nullptr);
	if (!window) {
		throw std::runtime_error("Failed to open GLFW window.");
		glfwTerminate();
	}
}
