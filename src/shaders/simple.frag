#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

layout(set=0,binding = 0) uniform PerFrameUBO {
	mat4 projView;
	vec3 viewPos;
} pfUBO[2];

layout(set=1,binding = 0) uniform GlobalUBO {
	vec3 lightPosition;
	vec3 light;
} gUBO[4];

layout(set=2, binding = 0) uniform sampler2D tex[100];

layout(push_constant) uniform PushConstant {
    uint frameIndex;
	mat4 model;
    ivec4 maps;
} pConst;

void main() {
	vec3 color = vec3(1.0);
	float specHighlight = 10.0;

	vec3 normal = normalize(inNormal);
	
	vec3 accColor = vec3(0.0);
	for(int i = 0; i < 4; i++) {
		vec3 lightDir = normalize(gUBO[i].lightPosition - inPosition);
		float diffStrength = max(dot(normal, lightDir), 0.0);
		vec3 diffuse = diffStrength * gUBO[i].light;

		vec3 viewDir = normalize(pfUBO[pConst.frameIndex].viewPos - inPosition);
		vec3 halfVec = normalize(viewDir + lightDir);	
		vec3 reflectDir = reflect(-lightDir, normal);  
		float spec = 0.0;
		if(specHighlight > 0.0) {
			spec = pow(max(dot(halfVec, normal), 0.0), specHighlight);
		}
		vec3 specular = spec * gUBO[i].light * (specHighlight + 2) / 6.283;  

		accColor += (diffuse + specular);
	}

	//outColor = vec4(accColor, 1.0);
	//outColor = texture(tex[0], inTexCoord);
	outColor = vec4(1.0);
}