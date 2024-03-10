#ifndef _ENGINEAPP_H_
#define _ENGINEAPP_H_
#include <chrono> 

struct GLFWwindow;

class App {
private:
	GLFWwindow* window;

	//mouse/keyboard states
    bool mousePressed[3]{};
    bool keyPressed[350]{};
    double cursorPos[2]{};
    double deltaCursorPos[2]{}; //change of cursor position from last tick

    std::chrono::steady_clock::time_point prevTime;
    uint64_t tickTime; //duration between prevTime to current tick time

public:
	//application callbacks
	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void mouse_callback(GLFWwindow* window, int button, int action, int mods);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void cursor_callback(GLFWwindow* window, double currX, double currY);

	void tick();

    App();
	App(GLFWwindow* w);
};

#endif