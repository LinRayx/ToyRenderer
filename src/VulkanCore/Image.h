#pragma once
#ifndef IMAGE_H
#define IMAGE_H
#include "Vulkan.h"
#include <memory>
using namespace std;

namespace Graphics {

	class Image
	{
		friend class RenderPass;

	public:
		Image(shared_ptr<Vulkan> _vulkan_ptr) : vulkan_ptr(_vulkan_ptr) {}
		~Image();

		void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

	private:
		shared_ptr<Vulkan> vulkan_ptr;
	};
}

#endif // !IMAGE_H