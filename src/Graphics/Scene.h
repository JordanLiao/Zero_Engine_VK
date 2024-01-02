#ifndef _SCENE_H_
#define _SCENE_H_


#include "VulkanRenderer.h"

#include "GLM/glm.hpp"

#include <vector>

class Instance;

class Scene {
public:
    Scene();
    Scene(VulkanRenderer* renderer);
    void render(glm::mat4 m, uint64_t appDuration);
    void addInstance(Instance*);
private:
    VulkanRenderer* renderer;
	std::vector<Scene*> subScenes;
	std::vector<Instance*> instances; //instances of objects on the current level
};

#endif