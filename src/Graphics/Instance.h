#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#include <string>

#include "GLM/glm.hpp"

struct Object;

class Instance {
public:
	Object* obj; //The graphical object that this instance is based on.
	int colorId;
	glm::mat4 model;
	glm::vec3 pos;
	std::string instName;

	Instance(Object*);
	void translate(glm::vec3 trans);
};

#endif
