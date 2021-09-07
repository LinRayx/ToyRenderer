#version 450
layout(location = 0) in vec2 inUV;

layout(constant_id = 0) const int SSAO_KERNEL_SIZE = 64;
layout(constant_id = 1) const float SSAO_RADIUS = 0.5;

layout(set = 0, binding = 0) uniform SceneParam
{
	mat4 view;
	mat4 proj;
} sParam;

layout(set = 0, binding = 1) uniform SSAOKernel
{
	vec4 samples[SSAO_KERNEL_SIZE];
} ssaoKernel;

layout(set = 0, binding = 2) uniform sampler2D samplerPositionDepth;
layout(set = 0, binding = 3) uniform sampler2D samplerNormal;
layout(set = 0, binding = 4) uniform sampler2D ssaoNoise;

layout(location = 0) out float outColor;

void main()
{
	// Get G-Buffer values
	vec3 fragPos = (sParam.view * vec4(texture(samplerPositionDepth, inUV).rgb, 1.0)).xyz;
	vec3 normal = normalize(texture(samplerNormal, inUV).rgb * 2.0 - 1.0);

	// ������С
	ivec2 texDim = textureSize(samplerPositionDepth, 0);
	ivec2 noiseDim = textureSize(ssaoNoise, 0);
	const vec2 noiseUV = vec2(float(texDim.x) / float(noiseDim.x), float(texDim.y) / (noiseDim.y)) * inUV;
	vec3 randomVec = texture(ssaoNoise, noiseUV).xyz * 2.0 - 1.0;

	// Create TBN matrix
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(tangent, normal);
	mat3 TBN = mat3(tangent, bitangent, normal);

	// Calculate occlusion value
	float occlusion = 0.0f;
	// remove banding
	const float bias = 0.025f;

	for (int i = 0; i < SSAO_KERNEL_SIZE; ++i) {
		vec3 samplePos = TBN * ssaoKernel.samples[i].xyz;
		samplePos = fragPos + samplePos * SSAO_RADIUS;

		// project
		vec4 offset = vec4(samplePos, 1.0f);
		offset = sParam.proj * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5f + 0.5f;

		float sampleDepth = -texture(samplerPositionDepth, offset.xy).w;
		float rangeCheck = smoothstep(0.0f, 1.0f, SSAO_RADIUS / abs(fragPos.z - sampleDepth));
		

		occlusion += (sampleDepth >= samplePos.z + bias ? 1.0f : 0.0f) * rangeCheck;
	}

	occlusion = 1.0 - (occlusion / float(SSAO_KERNEL_SIZE));

	outColor = occlusion;
}