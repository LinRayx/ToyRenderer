#pragma once
#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#include "Graphics.h"
#include "Vulkan.h"
#include <vector>
#include "CommandPool.h"
#include "RenderPass.h"
#include "Pipeline.h"
#include "Buffer.h"
#include "DescriptorSet.h"

using namespace std;

namespace Graphics
{

	class CommandBuffer : public Graphics
	{
		friend class CommandQueue;
	public:
		CommandBuffer(shared_ptr<Vulkan> vulkan_ptr, shared_ptr<CommandPool> cmdPool_ptr) : vulkan_ptr(vulkan_ptr), cmdPool_ptr(cmdPool_ptr)
		{
			drawCmdBuffers.resize(vulkan_ptr->swapchain.image_count);

			VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
			cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufAllocateInfo.commandPool = cmdPool_ptr->cmdPool;
			cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufAllocateInfo.commandBufferCount = static_cast<uint32_t>(drawCmdBuffers.size());

			vkAllocateCommandBuffers(vulkan_ptr->device.device, &cmdBufAllocateInfo, drawCmdBuffers.data());
		}

		~CommandBuffer()
		{
			vkFreeCommandBuffers(vulkan_ptr->device.device, cmdPool_ptr->cmdPool, static_cast<uint32_t>(drawCmdBuffers.size()), drawCmdBuffers.data());
		}

	private:
		shared_ptr<Vulkan> vulkan_ptr;
		shared_ptr<CommandPool> cmdPool_ptr;
	public:
		vector<VkCommandBuffer> drawCmdBuffers;
	};
}

#endif // !COMMAND_BUFFER_H