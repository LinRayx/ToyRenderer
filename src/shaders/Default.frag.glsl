#version 450

layout(push_constant) uniform PushConsts{
	layout(offset = 0)  vec4 color;
} pushConsts;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = pushConsts.color;
}