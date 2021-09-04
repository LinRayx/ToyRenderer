#version 450


layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inPos;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inTangent;


layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;
layout(location = 3) out vec4 outMetallicRoughness;

layout(set = 0, binding = 0) uniform SceneParam
{
	mat4 view;
	mat4 proj;
	float nearPlane;
	float farPlane;
}sParam;

layout(set = 2, binding = 0) uniform PbrParam
{
	bool HasAlbedoTex;
	bool HasMetallicTex;
	bool HasNormalTex;
	bool HasRoughnessTex;
	vec3 albedo;
	float metallic;
	float roughness;
	bool hasDiffuseTex;
} pParam;


layout(set = 2, binding = 1) uniform sampler2D albedoMap;
layout(set = 2, binding = 2) uniform sampler2D metallicMap;
layout(set = 2, binding = 3) uniform sampler2D normalMap;
layout(set = 2, binding = 4) uniform sampler2D roughnessMap;


float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f;
	return (2.0f * sParam.nearPlane * sParam.farPlane) / (sParam.farPlane + sParam.nearPlane - z * (sParam.farPlane - sParam.nearPlane));
}

vec3 calculateNormal()
{
	vec3 tangentNormal = texture(normalMap, inUV).xyz * 2.0 - 1.0;

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
	outAlbedo = texture(albedoMap, inUV);
	outMetallicRoughness.r = texture(roughnessMap, inUV).r;
	outMetallicRoughness.g = texture(metallicMap, inUV).r;
}