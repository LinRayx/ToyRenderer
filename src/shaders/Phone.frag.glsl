#version 450
layout (location = 0) in vec3 worldNormal;
layout (location = 1) in vec3 worldPos;
layout(location = 2) in vec2 texCoord;

layout (location = 0) out vec4 outColor;
//layout (location = 2) out vec4 swapChainColor;

layout (set = 0, binding = 1) uniform SceneParam
{
	vec3 viewPos;
	vec3 directLightDir;
	vec3 directLightColor;
}sParam;
layout(set = 1, binding = 1) uniform ModelParam{
	bool hasDiffuseTex;
	bool hasSpecularTex;
} mParam;
layout (set = 1, binding = 2) uniform sampler2D diffuse_tex;
layout(set = 1, binding = 3) uniform sampler2D specular_tex;

void main() {

	vec3 diffuseColor;
	if (mParam.hasDiffuseTex) {
		diffuseColor = texture(diffuse_tex, texCoord).rgb;
	}
	else {
		diffuseColor = vec3(1, 1, 1);
	}
	vec3 ambient = 0.3 * diffuseColor;

	vec3 norm = normalize(worldNormal);
	vec3 viewDir = normalize(sParam.viewPos-worldPos);
	
	vec3 lightDir = normalize(sParam.directLightDir);
	vec3 reflectDir = reflect(-lightDir, norm);

	float diff = max(dot(norm, -lightDir), 0.0);
	vec3 diffuse = diff * diffuseColor;
	
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);

	vec3 specularColor;
	if (mParam.hasSpecularTex) {
		specularColor = texture(diffuse_tex, texCoord).rgb;
	}
	else {
		specularColor = vec3(1, 1, 1);
	}

	vec3 specular = spec * specularColor;

	outColor = vec4( (diffuse + specular) * sParam.directLightColor + ambient, 1);
}