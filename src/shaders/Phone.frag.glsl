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

layout (set = 1, binding = 1) uniform sampler2D diffuse_tex;

void main() {

	vec3 diffuseColor = texture(diffuse_tex, texCoord).rgb;

	float specularStrength = 0.5;
	vec3 ambient = vec3(0.3f);
	vec3 norm = normalize(worldNormal);
	vec3 viewDir = normalize(sParam.viewPos-worldPos);
	
	vec3 lightDir = normalize(sParam.directLightDir);
	vec3 reflectDir = reflect(-lightDir, norm);

	float diff = max(dot(norm, -lightDir), 0.0);
	vec3 diffuse = diff * diffuseColor;
	
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * sParam.directLightColor;

	vec3 result = (diffuse + specular + ambient) * vec3(1, 1, 1);

	outColor = vec4(result, 1);

}