#ifndef _VULKANUNIFORMINFOS_H_
#define _VULKANUNIFORMINFOS_H_

#include "glm.hpp"

namespace VulkanUniformInfos {
    struct PerFrameUBO {
        alignas(16) glm::mat4 projView;
        alignas(16) glm::vec3 viewPos;
        alignas(16) glm::vec3 viewDir;
        float deltaT;
    };

    struct GlobalUBO {
        glm::vec3 lightPos;
        alignas(16) glm::vec3 light;
    };

    struct PBRConstant {
        alignas(4) uint32_t objIdx;
        alignas(4) uint32_t frameIdx;
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
