#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTexCoord;
layout(location = 3) out vec3 outTangent;
layout(location = 4) out vec3 outBitangent;

layout(set=0,binding = 0) uniform PerFrameUBO {
    mat4 projView;
    vec3 viewDir;
} pfUBO[2];

layout(push_constant) uniform PushConstant {
    uint frameIndex;
    mat4 model;
    ivec4 maps;
} pConst;

void main() {
    mat4 model = mat4(1.f);
    gl_Position = pfUBO[pConst.frameIndex].projView * vec4(inPosition, 1.0f);
    outPosition = vec3(model * vec4(inPosition, 1.0));
    outNormal = normalize(mat3(transpose(inverse(model))) * inNormal);
    outTexCoord = inTexCoord;
    outTangent = inTangent;
    outBitangent = inBitangent;
}