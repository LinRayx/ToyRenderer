#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inTangent;

//layout(location = 4) in vec3 inPrecomputeLT1;
//layout(location = 5) in vec3 inPrecomputeLT2;
//layout(location = 6) in vec3 inPrecomputeLT3;

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
//layout(location = 4) in vec3 inPrecomputeLT1;
//layout(location = 5) in vec3 inPrecomputeLT2;
//layout(location = 6) in vec3 inPrecomputeLT3;

void main()
{
	gl_Position = sParam.projection * sParam.view * mParam.model * vec4(inPos, 1.0f);

	outUV = inUV;

	// Vertex position in view space
	outPos = vec3(mParam.model * vec4(inPos, 1.0f));

	// Normal in view space
	mat3 normalMatrix = transpose(inverse(mat3(mParam.model)));
	outNormal = normalMatrix * inNormal;
	outTangent = vec4(normalMatrix * inTangent.xyz, inTangent.w);

	//outPrecomputeLT1 = inPrecomputeLT1;
	//outPrecomputeLT2 = inPrecomputeLT2;
	//outPrecomputeLT3 = inPrecomputeLT3;
}
