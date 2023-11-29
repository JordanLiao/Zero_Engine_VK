#ifndef _GRAPHICSBUFFERS_H_
#define _GRAPHICSBUFFERS_H_

#include <vector>
#include "GLM/glm.hpp"

struct IndexBuffer {
    std::vector<glm::ivec3> triangles;
};

struct VertexBuffer {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;
};

#endif