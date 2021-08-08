#version 450

layout (location = 0) in vec3 worldNormal;
layout (location = 1) in vec3 worldPos;
layout (location = 0) out vec4 outColor;


layout(binding = 1) uniform UniformPointLightObject {
	vec3 Position;
	vec3 Color;
	float density;
} point_light_ubo;

layout (binding = 2) uniform UniformConstantParamObject {
	vec3 viewPos;
} constant_param_ubo;

void main() {
	float specularStrength = 0.5;
	
	vec3 norm = normalize(worldNormal);
	vec3 viewDir = normalize(constant_param_ubo.viewPos-worldPos);
	
	vec3 lightDir = normalize(point_light_ubo.Position - worldPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * point_light_ubo.Color;
	
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * point_light_ubo.Color;;

	vec3 result = (diffuse + specular) * (1, 1, 1);

	outColor = vec4(result, 1);
}