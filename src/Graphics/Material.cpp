#include "Material.h"

Material::Material() {
	materialName = "default";
	ambient = glm::vec3(0.0f); //ka
	diffuse = glm::vec3(0.0f);  //kd
	specular = glm::vec3(0.0f);  //ks
	emissive = glm::vec3(0.0f);
	specularFocus = 0.f;  //ns
	opticalDensity = 1.f; //ni
	nonTransparency = 1.0f;  //d
	illum = 0;  //type of illumination
	diffuseTexture = 0; //map_kd
}
