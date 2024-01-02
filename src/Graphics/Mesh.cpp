#include "Mesh.h"

#include "../tools/ColorID.h"

Mesh::Mesh() {
    meshName = ColorID::getNewId();
}

Mesh::Mesh(std::string& name, Material* mtl, int indOff, int meshSize) {
    meshName = name;
    material = mtl;
    indexOffset = indOff;
    size = meshSize;
}

