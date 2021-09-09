#version 450

layout(location = 0) in vec3 inPos;
layout(location = 0) out vec4 outPos;
layout(location = 1) out vec3 outLightPos;

layout(set = 1, binding = 0) uniform ModelParam{
    mat4 model;
} mParam;

layout(set = 0, binding = 0) uniform SceneParam
{
    mat4 proj;
    vec3 lightPos;
}sParam;


layout(push_constant) uniform PushConsts
{
	mat4 view;
} pushConsts;

void main()
{
    outPos = mParam.model * vec4(inPos, 1.0);
    outLightPos = sParam.lightPos.xyz;
	gl_Position = sParam.proj * pushConsts.view * mParam.model * vec4(inPos, 1.0);
}