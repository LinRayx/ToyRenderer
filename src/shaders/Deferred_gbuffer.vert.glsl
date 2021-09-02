#version 450

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inTangent;

layout(set = 1, binding = 0) uniform ModelParam{
	mat4 model;
} mParam;

layout(set = 0, binding = 0) uniform SceneParam
{
	mat4 view;
	mat4 projection;
} sParam;


layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outPos;
layout(location = 2) out vec2 outUV;
layout(location = 3) out vec4 outTangent;


void main()
{
	gl_Position = sParam.projection * sParam.view * mParam.model * inPos;

	outUV = inUV;

	// Vertex position in view space
	outPos = vec3(sParam.view * mParam.model * inPos);

	// Normal in view space
	mat3 normalMatrix = transpose(inverse(mat3(sParam.view * mParam.model)));
	outNormal = normalMatrix * inNormal;
	outTangent = vec4(mat3(mParam.model) * inTangent.xyz, inTangent.w);
}
