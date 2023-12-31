#include "ResourceManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#include "Object.h"
#include "Mesh.h"
#include "Material.h"
#include "Cloth.h"
#include "../tools/MathConverter.h"

#include <iostream>
#include <stdexcept>

#ifdef USE_VULKAN
#include "VulkanBufferUtils.h"
#include "VulkanContext.h"
#include "VulkanResourceManager.h"

VulkanContext* ResourceManager::vulkanContext;
VulkanResourceManager* ResourceManager::vulkanResourceManager;
VkSampler ResourceManager::sampler2D;
VulkanCommandPool ResourceManager::vulkanTransferCmdPool;
VulkanCommandPool ResourceManager::vulkanGraphicsCmdPool;
#endif

std::unordered_map<std::string, uint32_t> ResourceManager::textureMap; //map of texture name to texture id

std::unordered_map<std::string, Object*> ResourceManager::objMap;
std::list<Object*> ResourceManager::objList;

bool ResourceManager::initialized;

void ResourceManager::init(VulkanContext* context, VulkanResourceManager* rManager) {
    vulkanContext = context;
    vulkanResourceManager = rManager;
    vulkanTransferCmdPool = VulkanCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                                                  vulkanContext->queueFamilyIndices.transferFamily.value(),
                                                  vulkanContext->logicalDevice);
    vulkanGraphicsCmdPool = VulkanCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                                                vulkanContext->queueFamilyIndices.graphicsFamily.value(),
                                                vulkanContext->logicalDevice);
    sampler2D = VulkanImageUtils::createSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, context);

    initialized = true;
}

void ResourceManager::cleanup() {
    vulkanTransferCmdPool.cleanUp();
    vulkanGraphicsCmdPool.cleanUp();
    initialized = false;
}

uint32_t ResourceManager::getTextureId(std::string& textureName){
    return uint32_t();
}

std::unordered_map<std::string, Material*> ResourceManager::getMaterialMap(std::string& materialMapName){
    return std::unordered_map<std::string, Material*>();
}

Image ResourceManager::loadImage(const std::string& path, Formats::ImageFormat format) {
    stbi_set_flip_vertically_on_load(true);
    
    VkFormat imgFormat;
    int comp = 0;
    if (format == Formats::R8G8B8A8 || format == Formats::R8G8B8) {
        imgFormat = VK_FORMAT_R8G8B8A8_SRGB;
        comp = 4;
    }
    /*else if(format == Formats::R8G8B8) {
        imgFormat = VK_FORMAT_R8G8B8_SRGB;
        comp = 3;
    }*/
    else if (format == Formats::R8G8) {
        imgFormat = VK_FORMAT_R8G8_SRGB;
        comp = 2;
    }
    else if (format == Formats::R8) {
        imgFormat = VK_FORMAT_R8_SRGB;
        comp = 1;
    }
    else {
        throw std::runtime_error("Cannot load this image format yet!");
    }

    stbi_uc* pixels = nullptr;
    Image image;
    pixels = stbi_load(path.c_str(), (int*)&image.width, (int*)&image.height, (int*)&image.channels, comp);
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VulkanImageUtils::createImage2D(image.data, image.width, image.height, imgFormat, VK_IMAGE_TILING_OPTIMAL, 
                                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanContext);

    int imageSize = image.width * image.height * comp;
    VulkanBuffer stagingBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vulkanContext);
    stagingBuffer.map();
    stagingBuffer.transferData(pixels, (size_t)imageSize);
    stagingBuffer.unmap();

    VulkanImageUtils::copyBufferToImage(stagingBuffer.vkBuffer, image.data.vkImage, image.width, image.height, 
                            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, vulkanGraphicsCmdPool);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = sampler2D;
    imageInfo.imageView = image.data.vkImageView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image.texId = vulkanResourceManager->addTexture2D(imageInfo);

    stagingBuffer.cleanUp();
    freeImageData((char*)pixels);

    return image;
}

void ResourceManager::freeImageData(char* pixels) {
    if (pixels == nullptr)
        return;
    stbi_image_free((stbi_uc*)pixels);
}

Material * ResourceManager::loadMaterial(const aiMaterial * mtl) {
    if (!initialized) {
        throw std::runtime_error("Resource Manager is not initialized!");
    }

	Material * mat = new Material();
	mat->materialName = std::string(mtl->GetName().C_Str());

	aiColor4D color;
	float val;
	int intVal;
	if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &color))
		mat->ambient = glm::vec3(color.r, color.g, color.b);

	if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &color))
		mat->diffuse = glm::vec3(color.r, color.g, color.b);

	if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &color))
		mat->specular = glm::vec3(color.r, color.g, color.b);

	if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &color))
		mat->emissive = glm::vec3(color.r, color.g, color.b);

	if (AI_SUCCESS == aiGetMaterialFloat(mtl, AI_MATKEY_SHININESS, &val))
		mat->specularFocus = val;

	if (AI_SUCCESS == aiGetMaterialFloat(mtl, AI_MATKEY_OPACITY, &val))
		mat->nonTransparency = val;

	if (AI_SUCCESS == aiGetMaterialInteger(mtl, AI_MATKEY_SHADING_MODEL, &intVal))
		mat->illum = intVal;

	return mat;
}

Object* ResourceManager::loadObject(const std::string& fPath) {
    if (!initialized) {
        throw std::runtime_error("Resource Manager is not initialized!");
    }

    std::string objName = std::string(getFileNameFromPath(fPath));
	// if this object was loaded previously
	if (objMap.find(objName) != objMap.end())
		return objMap[objName];

	Assimp::Importer imp;
	const aiScene* pScene = imp.ReadFile(fPath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | 
                                                aiProcess_FixInfacingNormals | aiProcess_CalcTangentSpace);
	if (pScene == nullptr) {
        std::cerr << "Error parsing " << fPath << ": " << imp.GetErrorString() << std::endl;
		return nullptr;
	}

	Object* obj = new Object(objName);

	VertexBuffer vertexBuffer; //vertices and normal values of meshes
	IndexBuffer indexbuffer; //triangle indices of meshes

	int vertexOffset = 0; //denotes the boundary in "vert" where a mesh begins
	int indexOffset = 0; //denotes the boundary in "indices" where a mesh begins

	//Parse the mesh data of the model/scene, including vertex data, bone data etc.
    obj->meshList.reserve(pScene->mNumMeshes);
	for (unsigned int i = 0; i < pScene->mNumMeshes; i++) {
		aiMesh* pMesh = pScene->mMeshes[i];
		std::string meshName = std::string(pMesh->mName.C_Str());

		processMeshVertices(pMesh, vertexBuffer);
		processMeshFaces(pMesh, indexbuffer.triangles, vertexOffset);
		//processMeshBones(pMesh, obj, vertToBone, obj->boneNameToID, vertexOffset);

		Mesh mesh(meshName, loadMaterial(pScene->mMaterials[pMesh->mMaterialIndex]), indexOffset, pMesh->mNumFaces * 3);
		obj->meshList.push_back(mesh);

		indexOffset = indexOffset + pMesh->mNumFaces * 3;
		vertexOffset = vertexOffset + pMesh->mNumVertices;
	}

	obj->root = new Node(pScene->mRootNode);
	//if(pScene->HasAnimations())
		//processAnimations(pScene, obj, obj->boneNameToID);

    obj->vkVertexBuffers = VulkanBufferUtils::createVertexBuffers(vertexBuffer, vulkanTransferCmdPool, vulkanContext);
    obj->vkIndexBuffer = VulkanBufferUtils::createIndexBuffer(indexbuffer, vulkanTransferCmdPool, vulkanContext);

	return obj;
}

Cloth* ResourceManager::createCloth(int width, int height, float restLength, float particleMass, float springK, float damperK) {
    std::vector<glm::vec4> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    positions.reserve(width * height);
    normals.reserve(width * height);
    texCoords.reserve(width * height);

    //std::vector<Cloth::SpringDamper> springDampers;
    std::vector<Cloth::SpringDamper> springDampers;
    std::vector<Cloth::ParticleState> particleStates;
    particleStates.reserve(width * height);

    float diagonalRestLength = std::sqrtf(2.f * restLength * restLength);
    glm::vec2 texCoord{0.f, 0.f};
    for (int i = 0; i < height; i++) {
        //glm::vec4 pos = { 0.f - (float)width / 2.f * restLength, (float)(height-i) * restLength, 0.f , 0.f};
        glm::vec4 pos = { 0.f - (float)width / 2.f * restLength, 5.f, (float)(i) * restLength , 0.f};
        for (int j = 0; j < width; j++) {
            positions.push_back(pos);
            pos += glm::vec4(restLength, 0.f, 0.f, 0.f);
            normals.push_back({ 0.f, 0.f, 1.f });
            texCoords.push_back(texCoord);
            texCoord += glm::vec2{1.f / (float)width, 1.f / (float)width};

            particleStates.push_back({{ 0.f, 0.f,0.f, 0.f }, { 0.f, 0.f,0.f, 0.f }});

            if (j != 0) //horizontal dampers
                springDampers.push_back({i* width + j - 1, i* width + j, restLength});
            if (i != 0) //vertical dampers
                springDampers.push_back({(i - 1)* width + j, i* width + j, restLength});
            if (i != 0 && j != 0) // diagonaldamper1
                springDampers.push_back({(i - 1)* width + j - 1, i* width + j, diagonalRestLength});
            if (i != 0 && j != width - 1) // diagonaldamper2
                springDampers.push_back({(i - 1)* width + j + 1, i* width + j, diagonalRestLength});

            /*if (j != 0) //horizontal dampers
                springDampers.push_back({ glm::ivec2{ i* width + j - 1, i* width + j} });
            if (i != 0) //vertical dampers
                springDampers.push_back({ glm::ivec2{ (i - 1)* width + j, i* width + j} });
            if (i != 0 && j != 0) // diagonaldamper1
                springDampers.push_back({ glm::ivec2{ (i - 1)* width + j - 1, i* width + j} });
            if (i != 0 && j != width - 1) // diagonaldamper2
                springDampers.push_back({ glm::ivec2{ (i - 1)* width + j + 1, i* width + j} });*/

            /*if (j != 0) //horizontal dampers
                springDampers.push_back({i* width + j - 1, i* width + j});
            if (i != 0) //vertical dampers
                springDampers.push_back({ (i - 1) * width + j, i * width + j});
            if (i != 0 && j != 0) // diagonaldamper1
                springDampers.push_back({(i - 1)* width + j - 1, i* width + j});
            if (i != 0 && j != width - 1) // diagonaldamper2
                springDampers.push_back({(i - 1)* width + j + 1, i* width + j});*/
        }
    }

    //init all the triangles
    IndexBuffer indexBuffer;
    indexBuffer.triangles.reserve((width - 1) * (height - 1));
    for (int i = 0; i < height - 1; i++) {
        for (int j = 0; j < width - 1; j++) {
            indexBuffer.triangles.push_back({ i * width + j, (i + 1) * width + j , i * width + j + 1 });
            indexBuffer.triangles.push_back({ (i + 1) * width + j, (i + 1) * width + j + 1, i * width + j + 1 });
        }
    }

    Cloth* cloth = new Cloth(restLength, particleMass, springK, damperK, {0.9f, 0.9f, 0.9f});
    cloth->numDampers = springDampers.size();
    cloth->numIndices = indexBuffer.triangles.size() * 3;
    cloth->numPart = positions.size();

    //springDamper SSBO
    cloth->springDamperSSBO = VulkanBufferUtils::createVulkanDataBuffer(springDampers.data(), springDampers.size() * sizeof(springDampers[0]),
                                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanTransferCmdPool, vulkanContext);
    //particle states SSBO
    cloth->particleSSBO = VulkanBufferUtils::createVulkanDataBuffer(particleStates.data(), particleStates.size() * sizeof(Cloth::ParticleState),
                                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanTransferCmdPool, vulkanContext);
    //particle pos vertexBuffer/SSBO
    VkDeviceSize bufferSize = sizeof(positions[0]) * positions.size();
    cloth->vkVertexBuffers.addBuffer(VulkanBufferUtils::createVulkanDataBuffer((void*)positions.data(), bufferSize,
                                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanTransferCmdPool, vulkanContext));
    bufferSize = sizeof(normals[0]) * normals.size();
    cloth->vkVertexBuffers.addBuffer(VulkanBufferUtils::createVulkanDataBuffer((void*)normals.data(), bufferSize,
                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanTransferCmdPool, vulkanContext));
    bufferSize = sizeof(texCoords[0]) * texCoords.size();
    cloth->vkVertexBuffers.addBuffer(VulkanBufferUtils::createVulkanDataBuffer((void*)texCoords.data(), bufferSize,
                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanTransferCmdPool, vulkanContext));
    cloth->vkIndexBuffer = VulkanBufferUtils::createIndexBuffer(indexBuffer, vulkanTransferCmdPool, vulkanContext);

    //cloth info desciptor
    cloth->descSet = vulkanResourceManager->addDescriptorSet(VulkanResourceManager::uboAlloc, 
                                                             VulkanRendererInfos::clothPipelineLayoutInfos[0],
                                                             vulkanResourceManager->clothDescSetLayouts[0]);
    cloth->clothStateUBO = VulkanBuffer(sizeof(Cloth::clothState),
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vulkanContext);
    cloth->clothStateUBO.map();

    Cloth::ClothState clothState{particleMass, springK, damperK};
    cloth->clothStateUBO.transferData(&clothState, sizeof(Cloth::ClothState));

    VkDescriptorAddressInfoEXT addressInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT };
    addressInfo.format = VK_FORMAT_UNDEFINED;
    addressInfo.range = cloth->clothStateUBO.hostSize;
    addressInfo.address = VulkanBufferUtils::getBufferDeviceAddress(cloth->clothStateUBO.vkBuffer, vulkanContext);
    VkDescriptorDataEXT descData{};
    descData.pUniformBuffer = &addressInfo;
    cloth->descSet.insertDescriptor(0, descData);

    addressInfo.range = cloth->springDamperSSBO.hostSize;
    addressInfo.address = VulkanBufferUtils::getBufferDeviceAddress(cloth->springDamperSSBO.vkBuffer, vulkanContext);
    descData = VkDescriptorDataEXT{};
    descData.pStorageBuffer = &addressInfo;
    cloth->descSet.insertDescriptor(1, descData);

    addressInfo.range = cloth->particleSSBO.hostSize;
    addressInfo.address = VulkanBufferUtils::getBufferDeviceAddress(cloth->particleSSBO.vkBuffer, vulkanContext);
    descData = VkDescriptorDataEXT{};
    descData.pStorageBuffer = &addressInfo;
    cloth->descSet.insertDescriptor(2, descData);

    addressInfo.range = cloth->vkVertexBuffers.buffers[0].hostSize;
    addressInfo.address = VulkanBufferUtils::getBufferDeviceAddress(cloth->vkVertexBuffers.vkBuffers[0], vulkanContext);
    descData = VkDescriptorDataEXT{};
    descData.pStorageBuffer = &addressInfo;
    cloth->descSet.insertDescriptor(3, descData);

    return cloth;
}

void ResourceManager::processMeshVertices(aiMesh* pMesh, VertexBuffer& buffer) {
	aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
    buffer.positions.resize(buffer.positions.size() + pMesh->mNumVertices);
    buffer.normals.resize(buffer.normals.size() + pMesh->mNumVertices);
    buffer.texCoords.resize(buffer.texCoords.size() + pMesh->mNumVertices);
	for (unsigned int j = 0; j < pMesh->mNumVertices; j++) {
		buffer.positions.push_back(glm::vec3(pMesh->mVertices[j].x, pMesh->mVertices[j].y, pMesh->mVertices[j].z));
		buffer.normals.push_back(glm::vec3(pMesh->mNormals[j].x, pMesh->mNormals[j].y, pMesh->mNormals[j].z));
		aiVector3D textCoord = pMesh->HasTextureCoords(0) ? pMesh->mTextureCoords[0][j] : Zero3D;
		buffer.texCoords.push_back(glm::vec2(textCoord.x, textCoord.y));
        buffer.tangents.push_back(glm::vec3(pMesh->mTangents[j].x, pMesh->mTangents[j].y, pMesh->mTangents[j].z));
        buffer.bitangents.push_back(glm::vec3(pMesh->mBitangents[j].x, pMesh->mBitangents[j].y, pMesh->mBitangents[j].z));
	}
}

void ResourceManager::processMeshFaces(aiMesh* pMesh, std::vector<glm::ivec3>& indices, int vertexOffset) {
	for (unsigned int j = 0; j < pMesh->mNumFaces; j++) {
		aiFace face = pMesh->mFaces[j];
		//need to offset the index by the number of vertices in the previous meshes because the indices for a mesh
		//are local to a mesh, so they all start from 0.
		indices.push_back(glm::ivec3(face.mIndices[0] + vertexOffset, face.mIndices[1] + vertexOffset, face.mIndices[2] + vertexOffset));
	}
}

void ResourceManager::processMeshBones(aiMesh* pMesh, Object* obj, std::vector<BonesForVertex>& vertToBone, 
										std::unordered_map<std::string, unsigned int>& boneNameToID, int vertexOffset) {
	for (unsigned int j = 0; j < pMesh->mNumBones; j++) {
		aiBone* aBone = pMesh->mBones[j];
		//if this bone has not been processed
		if (boneNameToID.find(aBone->mName.C_Str()) == boneNameToID.end()) {
			boneNameToID[aBone->mName.C_Str()] = (unsigned int)(obj->boneDataList.size()); //plus 1 to skip the 0th index because the default is 0
			Bone* boneData = new Bone(MathConverter::getMat4FromAiMatrix(pMesh->mBones[j]->mOffsetMatrix));
			obj->boneDataList.push_back(boneData);
		}

		int boneID = boneNameToID[aBone->mName.C_Str()];
		for (unsigned int k = 0; k < aBone->mNumWeights; k++) {
			vertToBone[aBone->mWeights[k].mVertexId + vertexOffset].addBoneWeight(boneID, aBone->mWeights[k].mWeight);
		}
	}
}

void ResourceManager::processAnimations(const aiScene* pScene, Object* obj, std::unordered_map<std::string, unsigned int>& boneNameToID) {
	for (unsigned i = 0; i < pScene->mNumAnimations; i++) {
		SkeletalAnimation* anim = new SkeletalAnimation(obj->root, pScene->mAnimations[i], boneNameToID);
		obj->animations.push_back(anim);
		obj->animationMap[pScene->mAnimations[i]->mName.C_Str()] = i;
	}
}

std::string ResourceManager::getFileNameFromPath(const std::string& fPath) {
    return fPath.substr(fPath.rfind("/") + 1, std::string::npos);
}

std::string ResourceManager::getFolderPath(const std::string& fPath) {
    return fPath.substr(0, fPath.rfind("/") + 1);
}

BonesForVertex::BonesForVertex() {
    for (size_t i = 0; i < MAX_NUM_BONE_PER_VERTEX; i++) {
        boneIDs[i] = 0;
        boneWeights[i] = 0.f;
    }
}

void BonesForVertex::addBoneWeight(unsigned int boneID, float boneWeight) {
    for (auto i = 0; i < MAX_NUM_BONE_PER_VERTEX; i++) {
        if (boneIDs[i] == boneID)
            return;
        if (boneWeights[i] == 0.f) {
            boneIDs[i] = boneID;
            boneWeights[i] = boneWeight;
            return;
        }
    }
    std::cerr << "ERROR: Vertex has more than " << MAX_NUM_BONE_PER_VERTEX << " of bones." << std::endl;
}
