#version 450
layout(set = 0, binding = 1) uniform SceneParam
{
	vec3 viewPos;
	vec3 directLightDir;
	vec3 directLightColor;
}sParam;

layout(set = 0, binding = 2) uniform samplerCube samplerCubeMap;

layout(location = 0) in vec3 inUVW;

layout(location = 0) out vec4 outFragColor;

void main()
{
	outFragColor = texture(samplerCubeMap, inUVW);
}