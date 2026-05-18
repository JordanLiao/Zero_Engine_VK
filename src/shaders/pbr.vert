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

struct ObjData {
	mat4 model;
	ivec4 maps; //PBR material maps IDs
};

layout(set=0,binding = 1) readonly buffer ObjDataBuffers {dsfsdfsd
	ObjData objData[];
};

layout(push_constant) uniform PushConstant {
    uint objIdx;
    mat4 projView;
	vec3 viewPos;
	vec3 viewDir;
} pConst;

void main() {
    mat4 model = objData[pConst.objIdx].model;

    gl_Position = pConst.projView * vec4(inPosition, 1.0f);
    outPosition = vec3(model * vec4(inPosition, 1.0));
    outNormal = normalize(mat3(transpose(inverse(model))) * inNormal);
    outTexCoord = inTexCoord;
    outTangent = inTangent;
    outBitangent = inBitangent;
}