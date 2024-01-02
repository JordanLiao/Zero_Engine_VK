#include "Scene.h"
#include "Instance.h"
#include "Object.h"
#include "Mesh.h"

#include "GLM/gtx/transform.hpp"

Scene::Scene() {}

Scene::Scene(VulkanRenderer* renderer) {
    this->renderer = renderer;
}

void Scene::render(glm::mat4 m, uint64_t appDuration) {
	double time = (double)appDuration / 1000.0; //time in terms of sec

    renderer->beginDrawCalls();
	for (int i = 0; i < instances.size(); i++) {
		Object* obj = instances[i]->obj;
        for (Mesh& m : obj->meshList) {
            renderer->draw(obj->vkIndexBuffer.vkBuffer, obj->vkVertexBuffers.vkBuffers.data(),
                           m.size, m.indexOffset, instances[i]->model, pbr.maps);
        }
	}
    renderer->submitDrawCalls();
}

void Scene::addInstance(Instance* instance) {
	instances.push_back(instance);
}

