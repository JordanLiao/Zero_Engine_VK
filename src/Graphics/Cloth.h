#ifndef _CLOTH_H_
#define _CLOTH_H_

#include "VulkanBuffer.h"
#include "VulkanDescriptorSet.h"

#include "GLM/glm.hpp"
#include <vector>

struct Cloth {
    struct SpringDamper {
        glm::ivec2 pair;
        float restLength;
        float padding;

        SpringDamper(int x, int y, float rl) { pair.x = x; pair.y = y; restLength = rl; }
    };

    struct ParticleState {
        glm::vec4 velocity;
        glm::vec4 force;
    };

    struct ClothState {
        float particleMass;
        float springK;
        float damperK;
        float dummy;
    };

    VulkanBufferArray vkVertexBuffers;
    VulkanBuffer vkIndexBuffer;
    VulkanBuffer springDamperSSBO;
    VulkanBuffer particleSSBO;
    VulkanBuffer clothStateUBO;

    VulkanDescriptorSet descSet;

	glm::vec3 color;
    ClothState clothState;
    float restLength;
    int numPart; //number of particles
    int numIndices; //number of triangle indices for drawing.
    int numDampers;

    Cloth();
	Cloth(float restLength, float particleMass, float springK, float damperK, glm::vec3 color);

    void cleanUp();
};

#endif