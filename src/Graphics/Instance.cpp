#include "Instance.h"
#include "Object.h"
#include "../tools/ColorID.h"

#include "GLM/gtx/transform.hpp"

Instance::Instance(Object* obj) {
    this->obj = obj;
    model = glm::mat4(1);
    colorId = ColorID::getNewId();
    instName = obj->objFileName + std::to_string(colorId);
}

void Instance::translate(glm::vec3 trans) {
    pos += trans;
    model = glm::translate(trans);
}
