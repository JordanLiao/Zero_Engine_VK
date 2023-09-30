#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

layout(set=0,binding = 0) uniform GlobalUniformBufferObject {
	vec3 lightPosition;
	vec3 light;
} gubo[4];

layout(set=1,binding = 0) uniform UniformBufferObject {
	mat4 projView;
	vec3 viewPos;
} ubo;

void main() {
	vec3 color = vec3(1.0);
	float specHighlight = 10.0;

	vec3 normal = normalize(inNormal);
	
	vec3 accColor = vec3(0.0);
	for(int i = 0; i < 4; i++) {
		vec3 lightDir = normalize(gubo[i].lightPosition - inPosition);
		float diffStrength = max(dot(normal, lightDir), 0.0);
		vec3 diffuse = diffStrength * gubo[i].light;

		vec3 viewDir = normalize(ubo.viewPos - inPosition);
		vec3 halfVec = normalize(viewDir + lightDir);	
		vec3 reflectDir = reflect(-lightDir, normal);  
		float spec = 0.0;
		if(specHighlight > 0.0) {
			spec = pow(max(dot(halfVec, normal), 0.0), specHighlight);
		}
		vec3 specular = spec * gubo[i].light * (specHighlight + 2) / 6.283;  

		accColor += (diffuse + specular);
	}

	outColor = vec4(accColor, 1.0);
}