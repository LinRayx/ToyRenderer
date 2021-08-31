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
		void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layoutCount = 1);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layoutCount = 1);
		void copyBufferToCubeImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layoutCount);
		void generateMipmap(VkCommandBuffer commandbuffer, VkImage image, VkImageBlit imageBlit, VkImageSubresourceRange mipSubRange);
		void Begin();
		void End();

		void CopyFrameBufferToImage(VkCommandBuffer cmd, string irradiance_attachment, string image_name, uint32_t dstBaseArrayLayer, int32_t dim, int32_t mipLevel = 0, int32_t viewport_width = -1, int32_t viewport_height = -1);

		void OffScreenBegin();
		void OffScreenEnd();
		// Put an image memory barrier for setting an image layout on the sub resource into the given command buffer
		void setImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkImageSubresourceRange subresourceRange,
			VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
		// Uses a fixed sub resource layout with first mip level and layer
		void setImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkImageAspectFlags aspectMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
		/** @brief Insert an image memory barrier into the command buffer */
		void insertImageMemoryBarrier(
			VkCommandBuffer commandbuffer,
			VkImage image,
			VkAccessFlags srcAccessMask,
			VkAccessFlags dstAccessMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			VkImageSubresourceRange subresourceRange);
	public:
		vector<VkCommandBuffer> drawCmdBuffers;
	};
}

#endif // !COMMAND_BUFFER_H