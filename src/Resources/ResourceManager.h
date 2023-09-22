#ifndef _RESOURCEMANAGER_H_
#define _RESOURCEMANAGER_H_

#include "VulkanContext.h"
#include "VulkanCommandPool.h"
#include "VulkanBufferUtils.h"
#include "Object.h"
#include "Material.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <unordered_map>
#include <list>
#include <queue>
#include <string>

#define MAX_NUM_BONE_PER_VERTEX 8

/*
	This struct records for each vertex which bones affect it and how much they affect it.
	The information is stored as Bone IDs that index into the Bone data list, and their
	corresponding weights on the vertex. This struct may store up to MAX_NUM_BONE_PER_VERTEX 
	number of bone ID to weight mappings.
*/
struct BonesForVertex {
	uint32_t boneIDs[MAX_NUM_BONE_PER_VERTEX];
	float boneWeights[MAX_NUM_BONE_PER_VERTEX];

	BonesForVertex();

	/*
		find the first slot among 0 to MAX_NUM_BONE_PER_VERTEX-1 that has a weight of 0, and
		store the BoneID and boneWeight in that location.
	*/
	void addBoneWeight(unsigned int boneID, float boneWeight);
};

class ResourceManager {
private:
    static bool initialized;
    static VulkanContext vulkanContext;
    static VulkanCommandPool vulkanCommandPool;

	//map of texture name to texture id
	static std::unordered_map<std::string, uint32_t> textureMap;
	/*mapping mtl "file" names to maps of mtl values.Design decision due to the fact that mtl file
	names are unique, whereas single mtl struct may not be.*/
	static std::unordered_map<std::string, std::unordered_map<std::string, Material*>*> mtlMapMap;

public:
	//mapping object names to objects
	static std::unordered_map<std::string, Object*> objMap;
	static std::list<Object*> objList;

	static void init(VulkanContext& context);
    static void cleanup();

	static uint32_t getTextureId(std::string& textureName);
	static std::unordered_map<std::string, Material*> getMaterialMap(std::string& materialMapName);

	static Material* loadMaterial(const aiMaterial * mtl);
	
	/*
		Load a FBX/OBJ file in the asset folder and return a reference to an untransformed static 3D object.
	*/
	static Object* loadObject(const char* fName);

	//extract the file name from the file path
	static std::string getFileNameFromPath(std::string& fPath);

	
	//extract the prefix folder path /a/b/ from full file path /a/b/file.ext
	static std::string getFolderPath(std::string& fPath);

private:
	//process and record a mesh's vertex attributes.
	static void processMeshVertices(aiMesh* pMesh, std::vector<glm::vec3>& vert, std::vector<glm::vec3>& norm, 
									std::vector<glm::vec2>& text);
	//process and record a mesh's trianges/faces
	static void processMeshFaces(aiMesh* pMesh, std::vector<glm::ivec3>& indices, int vertexOffset);
	//process a mesh's bones
	static void processMeshBones(aiMesh* pMesh, Object* obj, std::vector<BonesForVertex>& vertToBone, 
								std::unordered_map<std::string, unsigned int>& boneNameToID, int vertexOffset);

	static void processAnimations(const aiScene* pScene, Object* obj, std::unordered_map<std::string, unsigned int>& boneNameToID);
};

#endif

