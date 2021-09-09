#pragma once
#ifndef IMAGE_H
#define IMAGE_H
#include "Vulkan.h"
#include "CommandBuffer.h"
#include "VulkanCore/VulkanInitalizers.hpp"
#include <memory>
using namespace std;

namespace Graphics {

	class Image
	{
		friend class RenderPass;

	public:
		static Image* getInstance() {
			if (instance == NULL) {
				instance = new Image();
			}
			return instance;
		}
		void createImage(VkImageCreateInfo imageCreateInfo, VkImage& image, VkDeviceMemory& imageMemory);
		void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void createCubeImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void generateMipmap(VkImage image, uint32_t width, uint32_t height, int32_t mipLevel,
			shared_ptr<CommandBuffer> cmdBuf);
		VkImageView createImageView(VkImageViewCreateInfo view);
		VkImageView createImageView(VkImage image, VkImageViewType viewType, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t layoutCount = 1);
		VkSampler createSampler(VkSamplerCreateInfo samplerInfo);
		void generateMipmap(VkCommandBuffer commandbuffer, VkImage image, VkImageBlit imageBlit, VkImageSubresourceRange mipSubRange);
		void CopyFrameBufferToImage(VkCommandBuffer cmd, VkImage& irradiance_attachment, VkImage& image_name, uint32_t dstBaseArrayLayer, int32_t dim, int32_t mipLevel = 0, float viewport_width = -1, float viewport_height = -1);
		void CopyFrameBufferToImage2(VkCommandBuffer cmd, VkImage& resource, VkImage& attachment, int32_t dim, int face);

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
	private:

		Image() {}
		~Image();

		class Deletor {
		public:
			~Deletor() {
				if (Image::instance != NULL)
					delete Image::instance;
			}
		};
		static Deletor deletor;
		static Image* instance;
	};

}

#endif // !IMAGE_H