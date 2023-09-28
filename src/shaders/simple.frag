#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

layout(set=0,binding = 0) uniform GlobalUniformBufferObject {
    vec3 lightPosition;
    vec3 light;
} gubo;

layout(set=1,binding = 0) uniform UniformBufferObject {
    mat4 projView;
	vec3 viewPos;
} ubo;

void main() {
	vec3 color = vec3(1.0);
	float specHighlight = 10.0;

    vec3 normal = normalize(inNormal);
    
	vec3 lightDir = normalize(gubo.lightPosition - inPosition);
	lightDir = gubo.lightPosition;
    float diffStrength = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diffStrength * color;

    vec3 viewDir = normalize(ubo.viewPos - inPosition);
	vec3 halfVec = normalize(viewDir + lightDir);	
	vec3 reflectDir = reflect(-lightDir, normal);  
	float spec = 0.0;
	if(specHighlight > 0.0) {
	  spec = pow(max(dot(halfVec, normal), 0.0), specHighlight);
	}
	vec3 specular = spec * color * (specHighlight + 2) / 6.283;  

	outColor = vec4(diffuse + specular, 1.0);
}