#version 450
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUv;


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout (location = 0) out vec3 worldNormal;
layout (location = 1) out vec3 worldPos;

void main() {
//    mat4 model = {{0.92568, -0.23985, 0.29254, 0.00},
//{0.371, 0.72678, -0.57806, 0.00},
//{-0.07397, 0.64363, 0.76175, 0.00},
//{0.00, 0.00, 0.00, 1.00} };
//
//    mat4 view =  {{1.00, 0.00, -4.37114E-08, 1.31134E-07},
//{0.00, 1.00, 0.00, 0.00},
//{4.37114E-08, 0.00, 1.00, -3.00},
//{0.00, 0.00, 0.00, 1.00}};
//mat4 proj = {
// {1.81066, 0.00, 0.00, 0.00},
//{0.00, 2.41421, 0.00, 0.00},
//{0.00, 0.00, -1.002, -0.2002},
//{0.00, 0.00, -1.00, 0.00}};
    vec4 pos = vec4(inPosition, 1.0f);
    worldNormal = mat3(transpose(inverse(ubo.model))) * inNormal;
    worldPos = vec3(ubo.model * pos);
	gl_Position = ubo.proj * ubo.view *  ubo.model * pos;
}