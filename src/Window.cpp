#include "Window.h"

//#include "GLM/gtx/transform.hpp"
#include "GLFW/glfw3.h"

#include <iostream>
#include <stdexcept>

Window::Window(int width, int height, const char* windowName) {
	this->width = width;
	this->height = height;
    //projMat = glm::perspective(glm::radians(50.0f), (float)width / (float)height, 0.1f, 1000.0f);

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
