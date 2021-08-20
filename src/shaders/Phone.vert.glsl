#version 450
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;


layout(set = 1, binding = 0) uniform ModelParam {
    mat4 model;
} mParam;

layout (set = 0, binding = 0) uniform SceneParam
{
    mat4 view;
    mat4 proj;
}sParam;

layout (location = 0) out vec3 worldNormal;
layout (location = 1) out vec3 worldPos;

void main()
{
    vec4 pos = vec4(inPosition, 1.0);

    worldNormal = mat3(transpose(inverse(mParam.model))) * inNormal;
    worldPos = vec3(mParam.model * pos);

    gl_Position = sParam.proj * sParam.view * mParam.model * pos;

}