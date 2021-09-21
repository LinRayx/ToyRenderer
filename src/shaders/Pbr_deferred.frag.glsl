#version 450

#define MAX_POINTLIGHT_NUM 1

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outDiffuse;
layout(location = 1) out vec4 outSpecular;

struct PointLight
{
	vec3 position;
	vec4 color;
	float constant;
	float lineart;
	float quadratic;
};

layout(set = 0, binding = 0) uniform SceneParam
{
	vec3 viewPos;
	mat4 view;
	PointLight pl[MAX_POINTLIGHT_NUM];
	bool SSAO;
}sParam;

layout(set = 0, binding = 1) uniform sampler2D brdfLUT;
layout(set = 0, binding = 2) uniform samplerCube irradianceMap;
layout(set = 0, binding = 3) uniform samplerCube prefilteredMap;
layout(set = 0, binding = 4) uniform sampler2D gbuffer_positionDepthMap;
layout(set = 0, binding = 5) uniform sampler2D gbuffer_normalMap;
layout(set = 0, binding = 6) uniform sampler2D gbuffer_albedoMap;
layout(set = 0, binding = 7) uniform sampler2D gbuffer_metallicRoughnessMap;
layout(set = 0, binding = 8) uniform sampler2D ssaoMap;
layout(set = 0, binding = 9) uniform samplerCube lightShadowMap;

layout(set = 3, binding = 0) uniform DirctionLight
{
	vec4 cascadeSplits;
	mat4 cascadeViewProjMat[SHADOW_MAP_CASCADE_COUNT];
	mat4 inverseViewMat;
	vec3 lightDir;
#ifdef PRINT_CSM
	float CASCADEINDEX;
#endif
} dlParam;

layout(set = 3, binding = 1) uniform sampler2DArray csmShadowMap;

#define EPSILON 0.15
#define SHADOW_OPACITY 0.5

// xy * 0.5 + 0.5
const mat4 biasMat = mat4(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0
);

const float PI = 3.14159265359;
vec3 albedo;

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
	vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
	vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
	vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
	vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
	vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);

// From http://filmicgames.com/archives/75
vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

// NDF
float D_GGX(float dotNH, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return (alpha2) / (PI * denom * denom);
}

// GSF
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

// Fresnel Function
vec3 F_Schlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 F_SchlickR(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 prefilteredReflection(vec3 R, float roughness)
{
	const float MAX_REFLECTION_LOD = 9.0;
	float lod = roughness * MAX_REFLECTION_LOD;
	float lodf = floor(lod);
	float lodc = ceil(lod);
	vec3 a = textureLod(prefilteredMap, R, lodf).rgb;
	vec3 b = textureLod(prefilteredMap, R, lodc).rgb;
	return mix(a, b, lod - lodf);
}

vec3 specularContribution(vec3 L, vec3 LightColor, vec3 V, vec3 N, vec3 F0, float metallic, float roughness)
{
	vec3 H = normalize(V + L);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);

	vec3 color = vec3(0.0);

	if (dotNL > 0.0) {
		float D = D_GGX(dotNH, roughness);
		float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		vec3 F = F_Schlick(dotNV, F0);
		vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);
		vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
		color += (kD * albedo / PI + spec ) * dotNL * LightColor;
	}

	return color;
}

float calulateShadow(vec3 pos)
{
	// shadow, only enable in pointLight[0]
	vec3 lightVec = pos - sParam.pl[0].position;
	float dist = length(lightVec);

	float viewDistance = length(sParam.viewPos - pos);
	float diskRadius = (1.0 + (viewDistance / 256.0)) / 25.0;

	// Check if fragment is in shadow
	float shadow = 0.0;
	int samples = 20;
	for (int i = 0; i < 20; ++i) {
		float sampledDist = texture(lightShadowMap, lightVec + gridSamplingDisk[i] * diskRadius).r;
		shadow += (dist <= sampledDist + EPSILON) ? 1.0 : SHADOW_OPACITY;
	}
	shadow /= float(samples);
	return shadow;
}

float textureProj(vec4 shadowCoord, vec2 offset, uint cascadeIndex)
{
	float shadow = 1.0;
	float bias = 0.005;

	if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0) {
		float dist = texture(csmShadowMap, vec3(shadowCoord.st + offset, cascadeIndex)).r;
		if (shadowCoord.w > 0 && dist < shadowCoord.z - bias) {
			shadow = SHADOW_OPACITY;
		}
	}
	return shadow;
}

void main()
{

#ifdef PRINT_CSM
	float depth = texture(csmShadowMap, vec3(inUV, dlParam.CASCADEINDEX)).r;
	outDiffuse.rgb = vec3(depth);
	outSpecular.rgb = vec3(0.0f);
	return;
#endif

	float flag = texture(gbuffer_albedoMap, inUV).a;
	if (flag < 1) {
		outDiffuse = vec4(0, 0, 0, 1);
		outSpecular = vec4(0, 0, 0, 1);
		return;
	}
	vec3 pos = texture(gbuffer_positionDepthMap, inUV).rgb;
	albedo = texture(gbuffer_albedoMap, inUV).rgb;
	vec3 N = texture(gbuffer_normalMap, inUV).rgb * 2.0 - 1.0f;
	vec3 V = normalize(sParam.viewPos - pos);
	vec3 R = reflect(-V, N);

	// albedo = pow(texture(albedoMap, inUV).rgb, vec3(2.2));
	float metallic = texture(gbuffer_metallicRoughnessMap, inUV).r;
	float roughness = texture(gbuffer_metallicRoughnessMap, inUV).g;
	float ssao = texture(ssaoMap, inUV).r;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	// Specular contribution
	vec3 Lo = vec3(0.0);

	//for (int i = 0; i < 4; ++i) {
	//	vec3 L = normalize(sParam.pl[i].position - pos);
	//	vec3 color = sParam.pl[i].color.xyz * sParam.pl[i].color.w;
	//	Lo += specularContribution(L,color, V, N, F0, metallic, roughness);
	//}

	vec3 L = normalize(-dlParam.lightDir);
	vec3 color = vec3(3, 3, 3);
	Lo += specularContribution(L, color, V, N, F0, metallic, roughness);

	vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 reflection = prefilteredReflection(R, roughness).rgb;
	vec3 irradiance = texture(irradianceMap, N).rgb;

	vec3 diffuse = irradiance * albedo;

	vec3 F = F_SchlickR(max(dot(N, V), 0.0), F0, roughness);

	vec3 specular = reflection * (F * brdf.x + brdf.y);

	vec3 kD = 1.0 - F;
	kD *= 1.0 - metallic;

	vec3 ao = vec3(1.);
	if (sParam.SSAO)
		ao = ssao.rrr;
	vec3 ambient = (kD * diffuse + specular) * ao;

	vec4 viewPos = sParam.view * vec4(pos, 1);
	// Get cascade index for the current fragment's view position
	uint cascadeIndex = 0;
	for (uint i = 0; i < SHADOW_MAP_CASCADE_COUNT - 1; ++i) {
		if (viewPos.z < dlParam.cascadeSplits[i]) {
			cascadeIndex = i + 1;
		}
	}

	//float shadow = calulateShadow(pos);
	// Depth compare for shadowing
	vec4 shadowCoord = (biasMat * dlParam.cascadeViewProjMat[cascadeIndex]) * vec4(pos, 1.0);
	
	// shadowCoord.y = 1 - shadowCoord.y;
	float shadow = 1.0f;
	shadow = textureProj(shadowCoord / shadowCoord.w, vec2(0.0), cascadeIndex);
	outDiffuse = vec4(kD * diffuse * ao * shadow, 1);
	outSpecular = vec4((specular * ao + Lo) * shadow, 1);

#ifdef DEBUG_CSM
	switch (cascadeIndex) {
	case 0:
		outDiffuse.rgb *= vec3(1.0f, 0.25f, 0.25f);
		outSpecular.rgb *= vec3(1.0f, 0.25f, 0.25f);
		break;
	case 1:
		outDiffuse.rgb *= vec3(0.25f, 1.0f, 0.25f);
		outSpecular.rgb *= vec3(0.25f, 1.0f, 0.25f);
		break;
	case 2:
		outDiffuse.rgb *= vec3(0.25f, 0.25f, 1.0f);
		outSpecular.rgb *= vec3(0.25f, 0.25f, 1.0f);
		break;
	case 3:
		outDiffuse.rgb *= vec3(1.0f, 1.0f, 0.25f);
		outSpecular.rgb *= vec3(1.0f, 1.0f, 0.25f);
		break;
	}
#endif
}