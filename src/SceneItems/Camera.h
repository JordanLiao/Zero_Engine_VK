#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "GLM/glm.hpp"

enum cameraRole {
	engineDisplayCam,
	cameraSize
};

class Camera {
public:
	glm::mat4 proj, view;
    float near, far, fov, aspectRatio;
	glm::vec3 eyePos, lookDir, lookAtPoint, upVec, camLeft, camRight, camUp, camDown;
    bool usePerspective;

	Camera();
	Camera(glm::vec3 eyePos, glm::vec3 lookDir, glm::vec3 upVec, float near, float far, float fov, 
           float aspectRatio, bool usePerspective = true, float dimX = 0.f, float dimY = 0.f);

	void translate(glm::vec3 translation);
	void rotate(glm::vec3 axis, float rad);
	
    //A special case of translation that is only along the look direction
    void zoom(float distance);
};

#endif
