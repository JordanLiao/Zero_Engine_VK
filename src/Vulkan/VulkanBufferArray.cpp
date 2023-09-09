#include "VulkanBufferArray.h"

VulkanBufferArray::VulkanBufferArray() {}

VulkanBufferArray::~VulkanBufferArray() {
	cleanup();
}

void VulkanBufferArray::cleanup() {
	for (VulkanBuffer* buffer : buffers) {
		buffer->cleanup();
	}
}
