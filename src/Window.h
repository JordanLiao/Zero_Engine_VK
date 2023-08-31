#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "GLFW/glfw3.h"

class Window {
public:
	int width, height;
	const char* windowName;
	GLFWwindow* window;

	Window(int initWidth, int initHeight, const char* windowName);

private:

};

#endif