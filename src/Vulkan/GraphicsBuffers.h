#ifndef _GRAPHICSBUFFERS_H_
#define _GRAPHICSBUFFERS_H_

#include <vector>
#include "GLM/glm.hpp"

struct IndexBuffer {
    std::vector<glm::ivec3> triangles;
};

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