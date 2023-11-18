
#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <vector>
#include <string>
#include <unordered_map>

#include "SkeletalAnimation.h"
#include "Node.h"
#include "Bone.h"

#include "../Vulkan/VulkanBuffer.h"
#include "../Vulkan/VulkanBufferArray.h"

struct Mesh;
struct Material;

struct Object{
	std::string objFileName;
	glm::mat4 model;
    std::vector<Mesh> meshList; //might want to use a different data structure for efficiency.

    VulkanBufferArray vkVertexBuffers;
    VulkanBuffer vkIndexBuffer;
	
    Node* root; //the root node of this Object's scene graph; the root may not have any mesh

	std::vector<Bone*> boneDataList; //List of bones for bones transformation matrices. 
	std::unordered_map<std::string, unsigned int> boneNameToID; //IMPORTANT: boneID is 1th indexed.
	std::vector<SkeletalAnimation*> animations;
	std::unordered_map<std::string, std::size_t> animationMap; //maps animation name to its index

    Object();
	Object(std::string& objName, glm::mat4 m = glm::mat4(1));
	void cleanUp();
};

#endif

