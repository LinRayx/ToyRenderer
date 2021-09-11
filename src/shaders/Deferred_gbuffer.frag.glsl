#version 450

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inPos;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inTangent;
//layout(location = 4) in vec3 inPrecomputeLT1;
//layout(location = 5) in vec3 inPrecomputeLT2;
//layout(location = 6) in vec3 inPrecomputeLT3;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;
layout(location = 3) out vec4 outMetallicRoughness;
//layout(location = 4) out vec4 outIndirectColor;

layout(set = 0, binding = 1) uniform SceneParam
{
	float nearPlane;
	float farPlane;
}sParam;

layout(set = 2, binding = 0) uniform PbrParam
{
	vec3 albedo;
	float metallic;
	float roughness;
	// vec3 preComputeL[9];
} pParam;


#ifdef BINDING_ALBEDO
layout(set = 2, binding = BINDING_ALBEDO) uniform sampler2D albedoMap;
#endif
#ifdef BINDING_METALLIC
layout(set = 2, binding = BINDING_METALLIC) uniform sampler2D metallicMap;
#endif
#ifdef BINDING_NORMAL
layout(set = 2, binding = BINDING_NORMAL) uniform sampler2D normalMap;
#endif
#ifdef BINDING_ROUGHNESS
layout(set = 2, binding = BINDING_ROUGHNESS) uniform sampler2D roughnessMap;
#endif

float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f;
	return (2.0f * sParam.nearPlane * sParam.farPlane) / (sParam.farPlane + sParam.nearPlane - z * (sParam.farPlane - sParam.nearPlane));
}

vec3 calculateNormal()
{
	vec3 tangentNormal = vec3(0, 0, 1);
#ifdef BINDING_NORMAL
		tangentNormal = texture(normalMap, inUV).xyz * 2.0 - 1.0;
#endif
	vec3 N = normalize(inNormal);
	vec3 T = normalize(inTangent.xyz);
	vec3 B = normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);
	return normalize(TBN * tangentNormal);
}

void main()
{
	outPosition = vec4(inPos, linearDepth(gl_FragCoord.z));
	outNormal = vec4(calculateNormal() * 0.5 + 0.5, 1.0);
#ifdef BINDING_ALBEDO
		outAlbedo = texture(albedoMap, inUV);
#else
		outAlbedo = vec4(pParam.albedo, 1);
#endif
		outAlbedo.a = 1; // ±ê¼ÇÎ»
#ifdef BINDING_METALLIC
		outMetallicRoughness.r = texture(metallicMap, inUV).r;
#else
		outMetallicRoughness.r = pParam.metallic;
#endif
#ifdef BINDING_ROUGHNESS
		outMetallicRoughness.g = texture(roughnessMap, inUV).r;
#else
		outMetallicRoughness.g = pParam.roughness;
#endif
	//outIndirectColor = vec4(SHrecon(), 1.0f);

}