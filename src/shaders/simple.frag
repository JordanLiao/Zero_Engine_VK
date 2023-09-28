#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

layout(set=0,binding = 0) uniform GlobalUniformBufferObject {
    vec3 lightPosition;
    vec3 light;
} gubo;

void main() {
    vec3 lightDir = normalize(gubo.lightPosition - inPosition);
	//lightDir = gubo.lightPosition;
    vec3 normal = normalize(inNormal);
    float diffStrength = max(dot(normal, lightDir), 0.0);
    //float diffStrength = 1.f;

    outColor = vec4(gubo.light * diffStrength, 1.0);
}