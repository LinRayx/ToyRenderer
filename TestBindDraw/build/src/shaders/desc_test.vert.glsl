#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(set = 0, binding = 0) uniform TestData {
    float b1;
    float b2;
    float b3;
    vec2 b4;
} td;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragColor = vec3(td.b1, td.b2, td.b3);
//    fragColor = inColor;
}