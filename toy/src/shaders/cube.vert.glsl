#version 450
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUv;


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;


void main() {
    vec4 pos = vec4(inPosition, 1.0f);

	gl_Position = ubo.proj * ubo.view * ubo.model * pos;
}