#include "EngineApp.h"
#include "Camera.h"

#include "GLFW/glfw3.h"

GLFWwindow* EngineApp::window;

//mouse/keyboard states
bool EngineApp::mousePressed[3];
bool EngineApp::keyPressed[350];

double EngineApp::cursorPosition[2] = {0.0, 0.0};
double EngineApp::prevCursorPosition[2] = {0.0, 0.0};

cursorState EngineApp::currentCursorState = cursorState::idle;

std::chrono::steady_clock::time_point EngineApp::prevTime;
uint64_t EngineApp::tickDuration;

Camera* EngineApp::displayCam;

bool EngineApp::initialize(GLFWwindow* w) {
	window = w;
    displayCam = new Camera(glm::vec3(0.f, 0.f, 10.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f), 1.f, 1000.f, 50.f,
        16.f / 9.f, true);
  
	prevTime = std::chrono::steady_clock::now();
    tickDuration = 0;

	return true;
}

bool EngineApp::initializeObjects() {
	//currScene = new Scene();
	//shadowTest = new ShadowTester();

	return true;
}

void EngineApp::update() {
    std::chrono::steady_clock::time_point currTime = std::chrono::steady_clock::now();
    tickDuration = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(currTime - prevTime).count();
    prevTime = currTime;

	// Perform any necessary updates here
	if (currentCursorState == cursorState::freeCam) { //move camera in free cam
		if (keyPressed[GLFW_KEY_W])
			moveCamera(GLFW_KEY_W);
		if (keyPressed[GLFW_KEY_A])
			moveCamera(GLFW_KEY_A);
		if (keyPressed[GLFW_KEY_S])
			moveCamera(GLFW_KEY_S);
		if (keyPressed[GLFW_KEY_D])
			moveCamera(GLFW_KEY_D);
		if (keyPressed[GLFW_KEY_LEFT_SHIFT])
			moveCamera(GLFW_KEY_LEFT_SHIFT);
		if (keyPressed[GLFW_KEY_LEFT_CONTROL])
			moveCamera(GLFW_KEY_LEFT_CONTROL);
	}
}

void EngineApp::display() {
	currScene->render(glm::mat4(1), appDuration);
}

void EngineApp::moveCamera(int key) {
	if (key == GLFW_KEY_W)
        displayCam->translate(displayCam->lookDir * 0.07f);
	else if (key == GLFW_KEY_A)
        displayCam->translate(displayCam->camLeft * 0.05f);
	else if (key == GLFW_KEY_S)
        displayCam->translate(displayCam->lookDir * -0.07f);
	else if (key == GLFW_KEY_D)
        displayCam->translate(displayCam->camRight * 0.05f);
	else if (key == GLFW_KEY_LEFT_SHIFT)
        displayCam->translate(glm::vec3(0.f, 0.05f, 0.0f));
	else if (key == GLFW_KEY_LEFT_CONTROL)
        displayCam->translate(glm::vec3(0.f, -0.05f, 0.0f));
}

void EngineApp::cleanUp() {

}


void EngineApp::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS)
	{
		keyPressed[key] = true;
		switch (key) {
		    case GLFW_KEY_ESCAPE:
			    break;
		    case GLFW_KEY_F: //free cam that allows camera rotation and movement
			    if (currentCursorState == cursorState::idle) {
				    currentCursorState = cursorState::freeCam;
				    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			    }
			    else if (currentCursorState == cursorState::freeCam) { //revert free cam to idle fix cam
				    currentCursorState = cursorState::idle;
				    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			    }
                break;
		    default:
			    break;
		}
	}
	else if (action == GLFW_RELEASE) {
		keyPressed[key] = false;
	}
}

void EngineApp::cursor_callback(GLFWwindow* window, double currX, double currY) {
	double dx = currX - cursorPosition[0];
	double dy = currY - cursorPosition[1];

	switch (currentCursorState) {
	    case cursorState::freeCam : {
            displayCam->rotate(displayCam->camUp, (float)(dx * 0.003));
            displayCam->rotate(displayCam->camLeft, (float)(dy * -0.003));
            break;
	    }
	    default:
		    break;
	}

	cursorPosition[0] = currX;
	cursorPosition[1] = currY;
}

void EngineApp::mouse_callback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) { // if left mouse button pressed
		if(button == GLFW_MOUSE_BUTTON_LEFT) { // if left mouse button pressed
			mousePressed[0] = true;
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT) { // if right mouse button pressed
			mousePressed[1] = true;
		}
		else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {  // scroll wheel pressed
			mousePressed[2] = true;
			if (currentCursorState == cursorState::idle) { // only go to panning from idle state
				currentCursorState = cursorState::panning;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
		}
	}

	//release actions
	if (action == GLFW_RELEASE) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			mousePressed[0] = false;
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			mousePressed[1] = false;
		}
		else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
			mousePressed[2] = false;
			currentCursorState = cursorState::idle;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}

void EngineApp::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {

}

