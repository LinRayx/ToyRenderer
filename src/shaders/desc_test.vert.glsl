#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(binding = 0) uniform TestData {
    float b1;
    float b2;
    float b3;
    vec2 b4;
} td;

layout(binding = 1) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = mvp.proj * mvp.view * mvp.model * vec4(inPosition, 1.0);
    fragColor = vec3(td.b1, td.b2, td.b3);
//    fragColor = inColor;
}