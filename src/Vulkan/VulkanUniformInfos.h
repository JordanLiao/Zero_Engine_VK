#ifndef _VULKANUNIFORMINFOS_H_
#define _VULKANUNIFORMINFOS_H_

#include "GLM/glm.hpp"

namespace VulkanUniformInfos {
    struct PerFrameUBO {
        glm::mat4 projView;
        glm::vec3 viewPos;
        alignas(16) float deltaT;
    };

    struct GlobalUBO {
        glm::vec3 lightPos;
        alignas(16) glm::vec3 light;
    };

    struct PBRConstant {
        uint32_t frameIndex;
        alignas(16)
        glm::mat4 model;
        glm::ivec4 maps;
    };

    struct PhongConstant {
        uint32_t frameIndex;
        alignas(16) 
        glm::mat4 model;
    };

    struct DeltaTimeConstant {
        float dt;
    };
}

#endif
