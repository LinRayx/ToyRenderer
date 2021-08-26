#include "VulkanCore/CommandPool.h"

namespace Graphics
{
	CommandPool::CommandPool()
	{
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = Vulkan::getInstance()->device.queue_family_index;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		if (vkCreateCommandPool(Vulkan::getInstance()->device.device, &cmdPoolInfo, Vulkan::getInstance()->device.allocator, &cmdPool))
		{
			exit(1);
		}
	}

	CommandPool::~CommandPool()
	{
		vkDestroyCommandPool(Vulkan::getInstance()->device.device, cmdPool, Vulkan::getInstance()->device.allocator);
	}
	CommandPool* CommandPool::instance = NULL;

}