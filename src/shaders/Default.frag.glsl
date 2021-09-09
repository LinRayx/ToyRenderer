#version 450

layout(push_constant) uniform PushConsts{
	layout(offset = 0)  vec4 color;
} pushConsts;
layout(location = 0) in vec4 inPos;
layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(pushConsts.color.xyz, 1);
}