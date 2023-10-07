#include "ResourceManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#include "VulkanBufferUtils.h"
#include "Object.h"
#include "Mesh.h"
#include "../tools/MathConverter.h"

#include <iostream>
#include <stdexcept>


BonesForVertex::BonesForVertex(){
	for (size_t i = 0; i < MAX_NUM_BONE_PER_VERTEX; i++) {
		boneIDs[i] = 0;
		boneWeights[i] = 0.f;
	}
}

void BonesForVertex::addBoneWeight(unsigned int boneID, float boneWeight) {
	for (auto i = 0; i < MAX_NUM_BONE_PER_VERTEX; i++) {
		if (boneIDs[i] == boneID)
			return;
		if(boneWeights[i] == 0.f) {
			boneIDs[i] = boneID;
			boneWeights[i] = boneWeight;
			return;
		}
	}
	std::cerr << "ERROR: Vertex has more than " << MAX_NUM_BONE_PER_VERTEX << " of bones." << std::endl;
}

std::unordered_map<std::string, uint32_t> ResourceManager::textureMap; //map of texture name to texture id
//mapping mtl file names to maps of mtl values. Design decision due to the fact that mtl file
//names are unique, whereas single mtl value may not be.
std::unordered_map<std::string, std::unordered_map<std::string, Material*>*> ResourceManager::mtlMapMap;
//mapping object names to objects
std::unordered_map<std::string, Object*> ResourceManager::objMap;
//list to keep track of all the loaded objects
std::list<Object*> ResourceManager::objList;

bool ResourceManager::initialized;


VulkanContext* ResourceManager::vulkanContext;
VulkanCommandPool ResourceManager::vulkanCommandPool;

void ResourceManager::init(VulkanContext* context) {
    vulkanContext = context;
    vulkanCommandPool = VulkanCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                                          vulkanContext->queueFamilyIndices.transferFamily.value(),
                                          vulkanContext->logicalDevice);
    initialized = true;
}

void ResourceManager::cleanup() {
    vulkanCommandPool.cleanUp();
    initialized = false;
}

uint32_t ResourceManager::getTextureId(std::string& textureName){
    return uint32_t();
}

std::unordered_map<std::string, Material*> ResourceManager::getMaterialMap(std::string& materialMapName){
    return std::unordered_map<std::string, Material*>();
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

Object* ResourceManager::loadObject(const char* fPath) {
    if (!initialized) {
        throw std::runtime_error("Resource Manager is not initialized!");
    }

	std::string pathName = std::string(fPath);
    std::string objName = std::string(getFileNameFromPath(pathName));
	// if this object was loaded previously
	if (objMap.find(objName) != objMap.end())
		return objMap[objName];

	Assimp::Importer imp;
	const aiScene* pScene = imp.ReadFile(fPath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_FixInfacingNormals);
	if (pScene == nullptr) {
		printf("Error parsing '%s': '%s'\n", fPath, imp.GetErrorString());
		return nullptr;
	}

	Object* obj = new Object(objName);

	VertexBuffer vertexBuffer; //vertices and normal values of meshes
	std::vector<glm::vec2> text; //texture coordinates of meshes
	IndexBuffer indexbuffer; //triangle indices of meshes

	int vertexOffset = 0; //denotes the boundary in "vert" where a mesh begins
	int indexOffset = 0; //denotes the boundary in "indices" where a mesh begins

	//Parse the mesh data of the model/scene, including vertex data, bone data etc.
	for (unsigned int i = 0; i < pScene->mNumMeshes; i++) {
		aiMesh* pMesh = pScene->mMeshes[i];
		std::string meshName = std::string(pMesh->mName.C_Str());

		processMeshVertices(pMesh, vertexBuffer.positions, vertexBuffer.normals, text);
		processMeshFaces(pMesh, indexbuffer.triangles, vertexOffset);
		//processMeshBones(pMesh, obj, vertToBone, obj->boneNameToID, vertexOffset);

		Mesh* mesh = new Mesh(meshName, loadMaterial(pScene->mMaterials[pMesh->mMaterialIndex]), indexOffset, pMesh->mNumFaces * 3);
		obj->meshList.push_back(mesh);

		indexOffset = indexOffset + pMesh->mNumFaces * 3;
		vertexOffset = vertexOffset + pMesh->mNumVertices;
	}

	obj->root = new Node(pScene->mRootNode);
	//if(pScene->HasAnimations())
		//processAnimations(pScene, obj, obj->boneNameToID);

    obj->vulkanVertexBuffers = VulkanBufferUtils::createVertexBuffers(vertexBuffer, vulkanCommandPool, vulkanContext);
    obj->vulkanIndexBuffer = VulkanBufferUtils::createIndexBuffer(indexbuffer, vulkanCommandPool, vulkanContext);

	return obj;
}

void ResourceManager::processMeshVertices(aiMesh* pMesh, std::vector<glm::vec3>& vert, std::vector<glm::vec3>& norm, 
                                          std::vector<glm::vec2>& text) {
	aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
	for (unsigned int j = 0; j < pMesh->mNumVertices; j++) {
		vert.push_back(glm::vec3(pMesh->mVertices[j].x, pMesh->mVertices[j].y, pMesh->mVertices[j].z));
		//because "aiProcess_GenSmoothNormals" flag above we can always explect normals
		norm.push_back(glm::vec3(pMesh->mNormals[j].x, pMesh->mNormals[j].y, pMesh->mNormals[j].z));
		aiVector3D textCoord = pMesh->HasTextureCoords(0) ? pMesh->mTextureCoords[0][j] : Zero3D;
		text.push_back(glm::vec2(textCoord.x, textCoord.y));
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

std::string ResourceManager::getFileNameFromPath(std::string& fPath) {
    return fPath.substr(fPath.rfind("/") + 1, std::string::npos);
}

std::string ResourceManager::getFolderPath(std::string& fPath) {
    return fPath.substr(0, fPath.rfind("/") + 1);
}
