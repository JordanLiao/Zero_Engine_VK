#ifndef _IMAGE_H_
#define _IMAGE_H_

#ifdef USE_VULKAN
#include "VulkanImage.h"
#endif

#include <optional>

struct Image {
    uint32_t width, height, channels;
    std::optional<uint32_t> texId;
    VulkanImage data;
};

#endif