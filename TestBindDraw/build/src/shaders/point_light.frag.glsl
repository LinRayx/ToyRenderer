#version 450

layout (location = 0) out vec4 outColor;

layout(binding = 1) uniform UniformPointLightObject {
	vec3 Position;
	vec3 Color;
	float density;
} point_light_ubo;

void main() {
	outColor = vec4(point_light_ubo.Color.xyz, 1);
}