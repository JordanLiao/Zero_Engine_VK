#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "VulkanImage.h"

struct Image {
    uint32_t width, height, channels;
    VulkanImage data;
};

#endif