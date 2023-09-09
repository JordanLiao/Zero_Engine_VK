#ifndef _VERTEXBUFFER_H_
#define _VERTEXBUFFER_H_

#include <vector>
#include <glm.hpp>
#include <vector>

#define NUM_VERTEX_ATTRIBUTES 2

enum VertexAttributeIndex {
	position,
	normal,
};

struct VertexBuffer {
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
};

#endif