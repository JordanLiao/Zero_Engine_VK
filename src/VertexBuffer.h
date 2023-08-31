#ifndef _VERTEXBUFFER_H_
#define _VERTEXBUFFER_H_

#include <vector>
#include <glm.hpp>
#include <vector>

struct VertexBuffer {
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
};

#endif