#include "Object.h"
#include "Mesh.h"

#include "../tools/ColorID.h"

Object::Object() {
    objFileName =  "Object" + ColorID::getNewId();
    model = glm::mat4(1.0f);
    boneDataList.push_back(new Bone(glm::mat4(1.f)));
}

Object::Object(std::string& objName, glm::mat4 m) {
    objFileName = objName;
    model = m;
    boneDataList.push_back(new Bone(glm::mat4(1.f)));
}

void Object::cleanUp() {
    vkIndexBuffer.cleanUp();
    vkVertexBuffers.cleanUp();

    size_t size = boneDataList.size();
    for (size_t i = 0; i < size; i++)
        delete boneDataList[i];

    size = animations.size();
    for (size_t i = 0; i < size; i++)
        delete animations[i];

    delete root;
}