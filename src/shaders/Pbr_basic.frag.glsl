#version 450
layout(location = 0) in vec3 worldNormal;
layout(location = 1) in vec3 worldPos;
layout(location = 2) in vec2 texCoord;
layout(location = 0) out vec4 outColor;

layout (set = 0, binding = 1) uniform SceneParam
{
	vec3 viewPos;
	vec3 directLightDir;
	vec3 directLightColor;
}sParam;

layout(set = 1, binding = 1) uniform PbrParam
{
	float metallic;
	float roughness;
	bool hasDiffuseTex;
} pParam;

layout(set = 1, binding = 2) uniform sampler2D diffuse_tex;

const float PI = 3.14159265359;
vec3 albedo;

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
vec3 F_Schlick(float cosTheta, float metallic)
{
	vec3 F0 = mix(vec3(0.04), albedo, metallic);
	vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
	return F;
}

vec3 BRDF(vec3 L, vec3 V, vec3 N, float metallic, float roughness)
{
	// Precalculate vectors and dot products	
	vec3 H = normalize (V + L);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);
	float dotLH = clamp(dot(L, H), 0.0, 1.0);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);
	vec3 color = vec3(0.0);
		// Light color fixed
	vec3 lightColor = vec3(1.0);

	if (dotNL > 0.0)
	{
		float rroughness = max(0.05, roughness);
		float D = D_GGX(dotNH, roughness);
		float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		vec3 F = F_Schlick(dotNV, metallic);

		vec3 spec = D * F * G / (4.0 * dotNL * dotNV);
		color = spec * dotNL * lightColor;
	}
	
	return color;
}

void main()
{
	if (pParam.hasDiffuseTex) {
		albedo = texture(diffuse_tex, texCoord).rgb;
	}
	else {
		albedo = vec3(1.0f, 0.765557f, 0.336057f);
	}
	// albedo = vec3(1.0f, 0.765557f, 0.336057f);

	vec3 N = normalize(worldNormal);
	vec3 V = normalize(sParam.viewPos - worldPos);

	// Specular contribution
	vec3 Lo = vec3(0.0);
	Lo += BRDF(normalize(sParam.directLightDir), V, N, pParam.metallic, pParam.roughness);
		// Combine with ambient
	vec3 color = albedo * 0.02;
	color += Lo;

	// Gamma correct
	color = pow(color, vec3(0.4545));
	outColor = vec4(color, 1.0);
}