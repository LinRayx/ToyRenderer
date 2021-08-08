#pragma once
#ifndef GRAPHICS_H
#define GRAPHICS_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define VK_LOAD(FUNCTION_NAME) PFN_##FUNCTION_NAME p##FUNCTION_NAME = (PFN_##FUNCTION_NAME) glfwGetInstanceProcAddress(device->instance, #FUNCTION_NAME);
#define COUNT_OF(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

namespace Graphics
{
	enum class DescriptorType
	{
		UNIFORM
	};

	enum class StageFlag
	{
		VERTEX,
		FRAGMENT,
		ALL,
	};

	enum class BufferUsage
	{
		VERTEX_BUFFER,
		UNIFORM,
	};

	class Graphics
	{
	public:
		virtual ~Graphics() = default;

		static VkDescriptorType GetDescriptorType(DescriptorType type);

		static VkShaderStageFlags GetStageFlag(StageFlag flag);
	};
}

#endif // !GRAPHICS_H