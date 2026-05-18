#ifndef _ENGINEAPP_H_
#define _ENGINEAPP_H_

#include <unordered_map>
#include <chrono> 


class GLFWwindow;
class Camera;

enum class cursorState {
	idle,
	picking, // when G is pressed
	panning,  // when scroll wheel is hold down
	freeCam
};

class EngineApp {
private:
	static GLFWwindow* window;

	//mouse/keyboard states
	static bool mousePressed[3];
	static cursorState currentCursorState;
	static double cursorPosition[2];
	static double prevCursorPosition[2];
	static bool keyPressed[350];

    static std::chrono::steady_clock::time_point prevTime;
    static uint64_t tickDuration;

    static Camera* displayCam; //default camera entity 

public:
	//application callbacks
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_callback(GLFWwindow* window, int button, int action, int mods);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void cursor_callback(GLFWwindow* window, double currX, double currY);

	// Draw and Update functions
	static void update();
	static void display();

	//handler functions
	static void moveCamera(int key);

	//initialization and clean up functions
	static bool initialize(GLFWwindow* w);
	static bool initializeObjects();
	static void cleanUp();
};

#endif