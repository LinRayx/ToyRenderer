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
		CommandBuffer()
		{
			drawCmdBuffers.resize(Vulkan::getInstance()->swapchain.image_count);

			VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
			cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufAllocateInfo.commandPool = CommandPool::getInstance()->cmdPool;
			cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufAllocateInfo.commandBufferCount = static_cast<uint32_t>(drawCmdBuffers.size());

			vkAllocateCommandBuffers(Vulkan::getInstance()->device.device, &cmdBufAllocateInfo, drawCmdBuffers.data());
		}

		~CommandBuffer()
		{
			vkFreeCommandBuffers(Vulkan::getInstance()->device.device, CommandPool::getInstance()->cmdPool, static_cast<uint32_t>(drawCmdBuffers.size()), drawCmdBuffers.data());
		}

		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layoutCount = 1);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layoutCount = 1);

		void Begin();
		void End();
	private:

	public:
		vector<VkCommandBuffer> drawCmdBuffers;
	};
}

#endif // !COMMAND_BUFFER_H