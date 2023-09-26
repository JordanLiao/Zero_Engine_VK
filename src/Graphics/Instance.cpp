#include "Instance.h"
#include "Object.h"
#include "../tools/ColorID.h"

#include "GLM/gtx/transform.hpp"

Instance::Instance(Object* obj) {
    object = obj;
    model = glm::mat4(1);
    colorId = ColorID::getNewId();
    instanceName = obj->objFileName + std::to_string(colorId);
}

Instance::~Instance() {}

void Instance::translate(glm::vec3 trans) {
    pos += trans;
    model = glm::translate(trans);
}
