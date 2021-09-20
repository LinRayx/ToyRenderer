#version 450

layout (location = 0) in vec3 inPos;

layout(set = 1, binding = 0) uniform ModelParam
{
    mat4 model;
} mParam;

layout(set = 0, binding = 0) uniform CascadeParam
{
    mat4 cascadeProjView[SHADOW_MAP_CASCADE_COUNT];
} cParam;

layout(push_constant) uniform PushBlock
{
 uint cascadeIndex;
} pushConsts;
void main()
{
    gl_Position = cParam.cascadeProjView[pushConsts.cascadeIndex] * mParam.model * vec4(inPos, 1);
}