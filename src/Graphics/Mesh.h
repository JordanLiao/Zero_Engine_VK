#ifndef _MESH_H_
#define _MESH_H_

#include <string>

struct EngineMaterial;

struct Mesh {
	std::string meshName;
	EngineMaterial* material;

	//offset in terms of indices(not vertices) into vao to begin drawing this mesh
	uint32_t indexOffset;

	/*size is number of vertices referenced by this mesh's indices; NOTE that this is not the num of vertices
	in this mesh because identical vertices are joined; its value is 3 * num triangles*/
	uint32_t size; 

    Mesh();

	/*
		name: name of the mesh.
		mtl: reference to the material for this mesh.
		indexOff: offset in terms of indices(not vertices) into VAO to begin drawing this mesh
		meshSize: number of vertices referenced by this mesh's indices, 3 * num triangles
	*/
	Mesh(std::string& name, EngineMaterial* mtl, int indexOff, int meshSize);
};

#endif
