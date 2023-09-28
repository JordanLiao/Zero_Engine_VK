#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;

layout(set=1,binding = 0) uniform UniformBufferObject {
    mat4 projView;
} ubo;

void main() {
    mat4 model = mat4(1.f);
    gl_Position = ubo.projView * vec4(inPosition, 1.0f);
    outPosition = vec3(ubo.projView * vec4(inPosition, 1.0f));
    outNormal = normalize(mat3(transpose(inverse(model))) * inNormal);
}