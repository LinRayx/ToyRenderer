#version 450
layout(location = 0) in vec3 inPosition;

layout(set = 1, binding = 0) uniform ModelParam{
    mat4 model;
} mParam;

layout(set = 0, binding = 0) uniform SceneParam
{
    mat4 view;
    mat4 proj;
}sParam;

layout(location = 0) out vec3 outUVW;

void main()
{
    outUVW = inPosition;
    vec4 pos = sParam.proj * sParam.view *  mParam.model * vec4(inPosition.xyz, 1.0);
    gl_Position = pos.xyww;
}