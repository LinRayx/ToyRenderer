#version 450

layout(set = 0, binding = 1) uniform samplerCube samplerCubeMap;

layout(location = 0) in vec3 inUVW;

layout(location = 0) out vec4 outFragColor;

void main()
{
	vec3 color = texture(samplerCubeMap, inUVW).rgb;
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1 / 2.2));
	outFragColor = vec4(color, 1);
	
}