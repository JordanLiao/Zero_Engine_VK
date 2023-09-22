#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;

layout(set=0,binding = 0) uniform GlobalUniformBufferObject {
    vec4 light;
} gubo;

layout(set=1,binding = 0) uniform UniformBufferObject {
    mat4 projView;
} ubo;

void main() {
    gl_Position = ubo.projView * vec4(inPosition, 1.0f);
    fragColor = vec3(gubo.light);
}