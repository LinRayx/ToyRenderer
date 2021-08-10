#version 450
layout (location = 0) in vec3 worldNormal;
layout (location = 1) in vec3 worldPos;
layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 1) uniform SceneParam
{
	vec3 viewPos;
	vec3 directLightDir;
	vec3 directLightColor;
}sParam;

void main() {
	float specularStrength = 0.5;
	
	vec3 norm = normalize(worldNormal);
	vec3 viewDir = normalize(sParam.viewPos-worldPos);
	
	vec3 lightDir = normalize(sParam.directLightDir);
	vec3 reflectDir = reflect(-lightDir, norm);

	float diff = max(dot(norm, -lightDir), 0.0);
	vec3 diffuse = diff * sParam.directLightColor;
	
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * sParam.directLightColor;

	vec3 result = (diffuse + specular) * vec3(1, 1, 1);

	outColor = vec4(result, 1);
}