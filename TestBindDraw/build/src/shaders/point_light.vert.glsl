#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inUv;

layout (binding = 0) uniform UniformMvpParam {
	mat4 model;
	mat4 view;
	mat4 proj;
}mvp;

void main() {
	vec4 pos = vec4(inPosition, 1.0);
	gl_Position = mvp.proj * mvp.view * mvp.model * pos;
}