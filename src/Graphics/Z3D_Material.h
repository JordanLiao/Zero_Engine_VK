#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "GLM/glm.hpp"
#include <string>

struct EngineMaterial{
	std::string materialName;
	glm::vec3 ambient; //ka
	glm::vec3 diffuse;  //kd
	glm::vec3 specular;  //ks
	glm::vec3 emissive;  //emissive Coeficient
	float specularFocus ;  //ns
	float opticalDensity; //ni
	float nonTransparency;  //d
	uint32_t illum;  //type of illumination
	uint32_t diffuseTexture; //map_kd

	EngineMaterial();
};

struct PBRMaterial {
    glm::ivec4 maps;
};

#endif
