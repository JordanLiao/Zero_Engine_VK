
#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <vector>
#include <string>
#include <unordered_map>

#include "Mesh.h"
#include "SkeletalAnimation.h"
#include "Node.h"
#include "Bone.h"

#include "../Vulkan/VulkanBuffer.h"
#include "../Vulkan/VulkanBufferArray.h"

struct Object{
	std::string objFileName;

	glm::mat4 model;

    VulkanBufferArray vulkanVertexBuffers;
    VulkanBuffer vulkanIndexBuffer;
	
    Node* root; //the root node of this Object's scene graph; the root may not have any mesh

	/*list of bone info; 
	the indices of this list are the bone IDs. During animation the bones transformation
	matrices will be contructed using this list. */
	std::vector<Bone*> boneDataList;
	std::unordered_map<std::string, unsigned int> boneNameToID; //IMPORTANT: boneID is 1th indexed.

	std::vector<SkeletalAnimation*> animations;
	std::unordered_map<std::string, std::size_t> animationMap; //maps animation name to its index

	std::unordered_map<std::string, Material*> matMap; //map of all the materials this Object uses

	std::vector<Mesh*> meshList; //might want to use a different data structure for efficiencys

    Object();
	Object(std::string& objName, glm::mat4 m = glm::mat4(1));
	~Object();
};

#endif

