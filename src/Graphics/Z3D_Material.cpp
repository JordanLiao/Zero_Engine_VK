#include "Z3D_Material.h"

EngineMaterial::EngineMaterial() {
	materialName = "default";
	ambient = glm::vec3(0.0f); //ka
	diffuse = glm::vec3(0.0f);  //kd
	specular = glm::vec3(0.0f);  //ks
	emissive = glm::vec3(0.0f);
	specularFocus = 5.f;  //ns
	opticalDensity = 1.f; //ni
	nonTransparency = 1.0f;  //d
	illum = 0;  //type of illumination
	diffuseTexture = 0; //map_kd
}
