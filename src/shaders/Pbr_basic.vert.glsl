#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec4 inTangent;

layout(set = 1, binding = 0) uniform ModelParam{
    mat4 model;
} mParam;

layout(set = 0, binding = 0) uniform SceneParam
{
    mat4 view;
    mat4 proj;
}sParam;

layout(location = 0) out vec3 worldNormal;
layout(location = 1) out vec3 worldPos;
layout(location = 2) out vec2 texCoord;
layout(location = 3) out vec4 outTangent;

void main()
{
    vec4 pos = vec4(inPosition, 1.0);

    worldNormal = mat3(transpose(inverse(mParam.model))) * inNormal;
    outTangent = vec4(mat3(mParam.model) * inTangent.xyz, inTangent.w);
    worldPos = vec3(mParam.model * pos);
    texCoord = inTexCoord;

    gl_Position = sParam.proj * sParam.view * mParam.model * pos;

}