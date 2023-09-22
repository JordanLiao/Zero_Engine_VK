#ifndef _INSPECTIONPLANE_H_
#define _INSPECTIONPLANE_H_

#include "GLM/glm.hpp"
#include "Instance.h"

class InspectionPlane {
public:
	glm::mat4 proj;
	glm::mat4 view;
	glm::mat4 projView;
	glm::mat4 model;
	Object* obj;
	Instance* inst;
	InspectionPlane();
};

#endif