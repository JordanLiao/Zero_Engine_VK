#ifndef _WINDOW_H_
#define _WINDOW_H_

#define NOMINMAX
#include "GLM/glm.hpp"
//#include "GLFW/glfw3.h"

struct GLFWwindow;

class Window {
public:
	int width, height;
	const char* windowName;
	GLFWwindow* window;
    //glm::mat4* projMat;

	Window(int initWidth, int initHeight, const char* windowName);

private:

};

#endif