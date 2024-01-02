#include "Cloth.h"
#include "../Resources/GraphicsBuffers.h"

#include <iostream>

Cloth::Cloth() {
    clothState.particleMass = 0.00001f;
    clothState.springK = 0.f;
    clothState.damperK = 0.f;
    this->restLength = 0;
    this->color = glm::vec3(0.f);
    numIndices = 0;
    numPart = 0;
}

Cloth::Cloth(float restLength, float particleMass, float springK, float damperK, glm::vec3 color) {
    clothState.particleMass = std::max(particleMass, 0.00001f);
    clothState.springK = springK;
    clothState.damperK = damperK;
    this->restLength = restLength;
    this->color = color;
}

void Cloth::cleanUp() {
    vkVertexBuffers.cleanUp();
    vkIndexBuffer.cleanUp();
    springDamperSSBO.cleanUp();
    particleSSBO.cleanUp();
    clothStateUBO.cleanUp();
}
