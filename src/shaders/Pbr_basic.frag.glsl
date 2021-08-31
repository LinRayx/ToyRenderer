#version 450
layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 worldPos;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inTangent;

layout(location = 0) out vec4 outColor;


layout (set = 0, binding = 1) uniform SceneParam
{
	vec3 viewPos;
	vec3 directLightDir;
	vec3 directLightColor;
}sParam;
layout(set = 0, binding = 2) uniform sampler2D brdfLUT;
layout(set = 0, binding = 3) uniform samplerCube irradianceMap;
layout(set = 0, binding = 4) uniform samplerCube prefilteredMap;

layout(set = 1, binding = 1) uniform PbrParam
{
	float metallic;
	float roughness;
	bool hasDiffuseTex;
} pParam;


layout(set = 1, binding = 2) uniform sampler2D albedoMap;
layout(set = 1, binding = 3) uniform sampler2D metallicMap;
layout(set = 1, binding = 4) uniform sampler2D normalMap;
layout(set = 1, binding = 5) uniform sampler2D roughnessMap;
layout(set = 1, binding = 6) uniform sampler2D aoMap;


const float PI = 3.14159265359;
vec3 albedo;

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

vec3 specularContribution(vec3 L, vec3 V, vec3 N, vec3 F0, float metallic, float roughness)
{
	vec3 H = normalize(V + L);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);

	vec3 lightColor = vec3(1.0);

	vec3 color = vec3(0.0);

	if (dotNL > 0.0) {
		float D = D_GGX(dotNH, roughness);
		float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		vec3 F = F_Schlick(dotNV, F0);
		vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);
		vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
		color += (kD * albedo / PI + spec) * dotNL;
	}

	return color;
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

	vec3 N = normalize(inNormal);
	vec3 V = normalize(sParam.viewPos - worldPos);
	vec3 R = reflect(-V, N);

	albedo = pow(texture(albedoMap, inUV).rgb, vec3(2.2));
	float metallic = texture(metallicMap, inUV).r;
	float roughness = texture(roughnessMap, inUV).r;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	// Specular contribution
	vec3 Lo = vec3(0.0);
	vec3 L = normalize(sParam.directLightDir);
	// Lo += specularContribution(L, V, N, F0, metallic, roughness);

	vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 reflection = prefilteredReflection(R, roughness).rgb;
	vec3 irradiance = texture(irradianceMap, N).rgb;

	vec3 diffuse = irradiance * albedo;

	vec3 F = F_SchlickR(max(dot(N, V), 0.0), F0, roughness);

	vec3 specular = reflection * (F * brdf.x + brdf.y);

	vec3 kD = 1.0 - F;
	kD *= 1.0 - metallic;
	vec3 ambient = (kD * diffuse + specular) * texture(aoMap, inUV).rrr;

	vec3 color = ambient + Lo;
	
	float exposure = 4.5;
	float gamma = 2.2;

	// Tone mapping
	color = Uncharted2Tonemap(color * exposure);
	color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));

	// Gamma correction
	color = pow(color, vec3(1.0f / gamma));

	outColor = vec4(color, 1.0);
}