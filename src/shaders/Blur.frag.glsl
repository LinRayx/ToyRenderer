#version 450

layout(set = 0, binding = 0) uniform sampler2D samplerMap;

layout(location = 0) in vec2 inUV;

layout(location = 0) out float outFragColor;

#ifdef TEST
void test()
{

}
#endif

void main()
{
	const int blurRange = 2;
	int n = 0;
	vec2 texelSize = 1.0 / vec2(textureSize(samplerMap, 0));
	float result = 0.0;
	for (int x = -blurRange; x < blurRange; x++)
	{
		for (int y = -blurRange; y < blurRange; y++)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(samplerMap, inUV + offset).r;
			n++;
		}
	}
	outFragColor = result / (float(n));
}