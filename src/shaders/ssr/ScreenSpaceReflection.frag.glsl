#version 450

#define point2 vec2
#define point3 vec3
layout(location = 0) in vec2 inUV;
layout(set = 0, binding = 0) uniform CameraParam
{
	point3 csOrig;
	vec3 csDir;
	mat4 proj;
	float nearPlaneZ;
} cParam;
layout(set = 1, location = 0) uniform sample2D colorMap;
layout(set = 1, location = 1) uniform sample2D depthBuffer;

layout(location = 0) out vec4 outColor;

float zThickness = 0.1f;
float nearPlaneZ = 0.1f;
float stride = 0.1f;
float jitter = 0.1f;
const float maxSteps = 10f;
float maxDistance = 10f;

float distanceSquared(point2 a, point2 b)
{
	a -= b;
	return dot(a, a);
}

bool traceScreenSpaceRay1(
	point3 csOrig,
	vec3 csDir,
	mat4 proj,
	sampler2D csZBuffer,
	vec2 csZBufferSize,
	float zThickness,
	float nearPlaneZ,
	float stride,
	float jitter,
	const float maxSteps,
	float maxDistance,
	out point2 hitPixel,
	out point3 hitPoint
)
{
	float rayLength = ((csOrig.z + csDir.z * maxDistance) > nearPlaneZ) ? (nearPlaneZ - csOrig, z) / csDir.z : maxDistance;
	point3 csEndPoint = csOrig + csDir * rayLength;

	vec4 H0 = proj * vec4(csOrig, 1.0);
	vec4 H1 = proj * vec4(csEndPoint, 1.0);
	float k0 = 1.0 / H0.w, k1 = 1.0 / H1.w;

	point3 Q0 = csOrig * k0, Q1 = csEndPoint * k1;
	point2 P0 = H0.xy * k0, P1 = H1.xy * k1;
	P1 += vec2((distanceSquared(P0, P1) < 0.0001) ? 0.01, 0.0);
	vec2 delta = P1 - P0;
	bool permute = false;
	if (abs(delta.x) < abs(delta.y)) {
		permute = true;
		delta = delta.yx;
		P0 = P0.yx;
		P1 = P1.yx;
	}

	float stepDir = sign(delta.x);
	float invdx = stepDir / delta.x;

	vec3 dQ = (Q1 - Q0) * invdx;
	vec3 dk = (k1 - k0) * invdx;
	vec2 dP = vec2(stepDir, delta.y * invdx);

	dP *= stride; dQ *= stride; dk *= stride;
	P0 += dP * jitter; Q0 += dQ * jitter; k0 += dk * jitter;

	point3 Q = Q0;

	float  end = P1.x * stepDir;

	float k = k0, stepCount = 0.0, prevZMaxEstimate = csOrig.z;
	float rayZMin = prevZMaxEstimate, rayZMax = prevZMaxEstimate;
	float sceneZMax = rayZMax + 100;
	for (point2 P = P0;
		((P.x * stepDir) <= end) && (stepCount < maxSteps) &&
		((rayZMax < sceneZMax - zThickness) || (rayZMin > sceneZMax)) &&
		(sceneZMax != 0);
		P += dP, Q.z += dQ.z, k += dk, ++stepCount) {

		rayZMin = prevZMaxEstimate;
		rayZMax = (dQ.z * 0.5 + Q.z) / (dk * 0.5 + k);
		prevZMaxEstimate = rayZMax;
		if (rayZMin > rayZMax) {
			float t = rayZMin; rayZMin = rayZMax; rayZMax = t;
		}

		hitPixel = permute ? P.yx : P;
		// You may need hitPixel.y = csZBufferSize.y - hitPixel.y; here if your vertical axis
		// is different than ours in screen space
		sceneZMax = texelFetch(csZBuffer, int2(hitPixel), 0);
	}

	// Advance Q based on the number of steps
	Q.xy += dQ.xy * stepCount;
	hitPoint = Q * (1.0 / k);
	return (rayZMax >= sceneZMax - zThickness) && (rayZMin < sceneZMax);
}

void main()
{
	point2 hitPixel;
	point3 hitPoint;
	vec2 depthSize = textureSize(depthBuffer, 0);
	bool shot = traceScreenSpaceRay1(
		cParam.csOrig,
		cParam.csDir,
		cParam.proj,
		depthBuffer,
		depthSize,
		zThickness,
		cParam.nearPlaneZ,
		stride,
		jitter,
		maxSteps,
		maxDistance,
		hitPixel,
		hitPoint
	);

	if (shot) {
		vec2 colorSize = textureSize(colorMap, 0);
		hitPixel = hitPixel * (colorSize / depthSize);
		outColor = texture(colorMap, hitPixel);
	}
}