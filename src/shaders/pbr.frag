#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

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
    uint frameIdx;
	mat4 model;
    ivec4 maps;
} pConst;

const float PI = 3.14159265359;
const vec3 lightPositions[4] = {{-3.0, 4.0, 3.0}, {3.0, 4.0, 3.0}, {-3.0, -4.0, 3.0}, {3.0, -4.0, 3.0}};
const vec3 lightColors[4] = {{24.0, 24.0, 24.0},{24.0, 24.0, 24.0}, {24.0, 24.0, 24.0},{24.0, 24.0, 24.0}};

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

void main(){		
	vec3 albedo = pow(texture(tex[pConst.maps[0]], inTexCoord).rgb, vec3(2.2));
	float roughness = texture(tex[pConst.maps[2]], inTexCoord).r;
	float metallic = texture(tex[pConst.maps[3]], inTexCoord).r;

    vec3 N = normalize(inNormal);
    vec3 V = normalize(pfUBO[pConst.frameIdx].viewPos - inPosition);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	           
    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - inPosition);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - inPosition);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;        
        
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);        
        float G = GeometrySmith(N, V, L, roughness);      
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;  
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);                
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    }   
  
    vec3 ambient = vec3(0.01) * albedo;
    vec3 color = ambient + Lo;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
   
    outColor = vec4(color, 1.0);
    //outColor = vec4(metallic, metallic, metallic, 1.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0){
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  