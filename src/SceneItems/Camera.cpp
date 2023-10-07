#include "Camera.h"

#include "GLM/gtx/transform.hpp"

#include <stdexcept>

Camera::Camera() : Camera(glm::vec3(0.f, 0.f, 10.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.0f, 0.f), 
                          1.0f, 1000.f, 50.f, 16.f / 9.f, true) {}

Camera::Camera(glm::vec3 eyePos, glm::vec3 lookDir, glm::vec3 upVec, float near, float far, float fov, 
               float aspectRatio, bool usePerspective, float dimX, float dimY) {
    if(!usePerspective && (dimX == 0.f || dimY == 0.f))
        throw std::runtime_error("Dimension for orthographic projection cannot be 0.0f");

	this->eyePos = eyePos;
	this->lookDir = glm::normalize(lookDir);
	lookAtPoint = eyePos + lookDir;
	this->upVec = glm::normalize(upVec);

	camLeft = glm::normalize(glm::cross(upVec, lookAtPoint - eyePos));
	camRight = camLeft * -1.0f;
	camUp = glm::normalize(glm::cross(lookDir, camLeft));
	camDown = camUp * -1.0f;
	view = glm::lookAt(eyePos, lookAtPoint, upVec);

    this->near = near;
    this->far = far;
    this->fov = fov;
    this->aspectRatio = aspectRatio;
    this->usePerspective = usePerspective;

    if(usePerspective)
        proj = glm::perspective(glm::radians(fov), aspectRatio, near, far);
    else 
        proj = glm::ortho(-(dimX / 2.f), dimX / 2.f, -(dimY / 2.f), dimY / 2.f, near, far);
}

void Camera::translate(glm::vec3 translation) {
	eyePos += translation;
	lookAtPoint = eyePos + lookDir;
	view = glm::lookAt(eyePos, lookAtPoint, upVec);
}

void Camera::rotate(glm::vec3 axis, float rad) {
	glm::vec3 temp = glm::normalize(glm::vec3(glm::vec4(lookDir, 0.0f) * glm::rotate(rad, axis)));

	if (glm::length(glm::dot(temp , upVec)) < 0.98f) //rotated lookDir should not be too close to the upVec
		lookDir = temp;

	lookAtPoint = eyePos + lookDir;
	camLeft = glm::normalize(glm::cross(upVec, lookDir));
	camRight = camLeft * -1.0f;
	camUp = glm::normalize(glm::cross(lookDir, camLeft));
	camDown = camUp * -1.0f;
	view = glm::lookAt(eyePos, lookAtPoint, upVec);
}

void Camera::zoom(float distance) {
	eyePos += (distance * lookDir);
	lookAtPoint = eyePos + lookDir;
	view = glm::lookAt(eyePos, lookAtPoint, upVec);
}



