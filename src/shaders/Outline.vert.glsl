#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(set = 1, binding = 0) uniform ModelParam{
    mat4 model;
} mParam;

layout(set = 0, binding = 0) uniform SceneParam
{
    mat4 view;
    mat4 proj;
}sParam;


void main()
{
    vec4 pos = vec4(inPosition.xyz + inNormal * 0.025, 1.0);
    gl_Position = sParam.proj * sParam.view * mParam.model * pos;
}