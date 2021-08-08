#pragma once
#ifndef COMMAND_POOL_H
#define COMMAND_POOL_H

#include "Graphics.h"
#include "Vulkan.h"
#include <memory>

namespace Graphics
{
	class CommandPool : public Graphics
	{
		friend class CommandBuffer;
	public:
		CommandPool(std::shared_ptr<Vulkan> vulkan_ptr) : vulkan_ptr(vulkan_ptr)
		{
			VkCommandPoolCreateInfo cmdPoolInfo = {};
			cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolInfo.queueFamilyIndex = vulkan_ptr->device.queue_family_index;
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			if (vkCreateCommandPool(vulkan_ptr->device.device, &cmdPoolInfo, vulkan_ptr->device.allocator, &cmdPool))
			{
				exit(1);
			}
		}
		~CommandPool()
		{
			vkDestroyCommandPool(vulkan_ptr->device.device, cmdPool, vulkan_ptr->device.allocator);
		}
	private:
		std::shared_ptr<Vulkan> vulkan_ptr;
		VkCommandPool cmdPool;
	};
}

#endif // !COMMAND_POOL_H
