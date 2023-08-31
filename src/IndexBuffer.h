#ifndef _INDEXBUFFER_H_
#define _INDEXBUFFER_H_

#include <vector>
#include <glm.hpp>
#include <vector>

struct IndexBuffer {
	std::vector<glm::ivec3> triangles;
};

#endif