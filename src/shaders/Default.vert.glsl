#version 450
layout(location = 0) in vec3 inPos;


layout(set = 1, binding = 0) uniform ModelParam{
    mat4 model;
} mParam;

layout(set = 0, binding = 0) uniform SceneParam
{
    mat4 view;
    mat4 proj;
}sParam;

layout(location = 0) out vec4 outPos;

void main()
{
    outPos = sParam.proj * sParam.view * mParam.model * vec4(inPos, 1.0f);
	gl_Position = sParam.proj * sParam.view * mParam.model * vec4(inPos, 1.0f);
}