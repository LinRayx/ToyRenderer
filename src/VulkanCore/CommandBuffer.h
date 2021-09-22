#pragma once
#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#include "Graphics.h"
#include "Vulkan.h"
#include <vector>
#include "CommandPool.h"
#include "RenderPass.h"

using namespace std;

namespace Graphics
{

	class CommandBuffer : public Graphics
	{
		friend class CommandQueue;
	public:
		CommandBuffer()
		{
			std::cout << "CommandBuffer()" << std::endl;
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
			std::cout << "~CommandBuffer()" << std::endl;
			vkFreeCommandBuffers(Vulkan::getInstance()->device.device, CommandPool::getInstance()->cmdPool, static_cast<uint32_t>(drawCmdBuffers.size()), drawCmdBuffers.data());
		}

		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layoutCount = 1);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layoutCount = 1);
		void copyBufferToCubeImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layoutCount);
		

		void Begin();
		void End();

		void DefaultBegin();
		void DefaultEnd();

		void DeferredBegin(RenderPassType type = RenderPassType::DEFERRED);
		void DeferredEnd();

		void OffScreenBegin();
		void OffScreenEnd();
		

		void ShadowBegin(RenderPassType type, int index, int fbIndex = -1);
		void ShadowEnd(int index);

	public:
		vector<VkCommandBuffer> drawCmdBuffers;
	};
}

#endif // !COMMAND_BUFFER_H