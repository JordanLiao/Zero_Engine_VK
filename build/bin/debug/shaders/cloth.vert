#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTexCoord;

layout(set=0,binding = 0) uniform PerFrameUBO {
    mat4 projView;
    vec3 viewDir;
} pfUBO[2];

layout(push_constant) uniform PushConstant {
    uint frameIdx;
    mat4 model;
} pConst;

void main() {
    gl_Position = pfUBO[pConst.frameIdx].projView * pConst.model * vec4(inPosition.xyz, 1.0f);
    outPosition = vec3(pConst.model * vec4(inPosition.xyz, 1.0));
    outNormal = normalize(mat3(transpose(inverse(pConst.model))) * inNormal);
    outTexCoord = inTexCoord;
}