#version 450
layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform sampler2D diffuseMap;
layout(set = 0, binding = 1) uniform sampler2D specularMap;

layout(location = 0) out vec4 outColor;

// From http://filmicgames.com/archives/75
vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

void main()
{
	vec3 diffuse = texture(diffuseMap, inUV).rgb;
	vec3 specular = texture(specularMap, inUV).rgb;

	vec3 color = diffuse + specular;
	float exposure = 4.5;
	float gamma = 2.2;

	// Tone mapping
	color = Uncharted2Tonemap(color * exposure);
	color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));

	// Gamma correction
	color = pow(color, vec3(1.0f / gamma));

	outColor = vec4(color, 1.0);
}