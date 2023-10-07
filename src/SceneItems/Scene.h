#ifndef _SCENE_H_
#define _SCENE_H_

#include "LightSource.h"

#include "GLM/glm.hpp"

#include <vector>

class Instance;

class Scene {
public:
    Scene();
    void render(glm::mat4 m, uint64_t appDuration);
    void translate(glm::vec3);
    void rotate(glm::mat4);
    void scale(float);
    void addInstance(Instance*);

private:
	//scene transforms
	glm::vec3 translation;
	glm::mat4 rotation;
	float scaleAmount;
	glm::mat4 model;

	LightSource* light; //may want to make this an vector in the futrue to have multiple light sources
	std::vector<Scene*> subScenes;
	std::vector<Instance*> instances; //instances of objects on the current level
};

#endif