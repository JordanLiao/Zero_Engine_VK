#include <iostream>
#include "Window.h"
#include "VulkanContext.h"

int main(int argc, char* argv[]) {
	Window window(900, 600, "Zero Engine VK");
	VulkanContext vulkanContext(window.window);
	vulkanContext.mainLoop();
}