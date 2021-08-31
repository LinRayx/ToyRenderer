#pragma once
#ifndef IMAGE_H
#define IMAGE_H
#include "Vulkan.h"
#include "CommandBuffer.h"
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

		void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void createCubeImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void generateMipmap(VkImage image, uint32_t width, uint32_t height, uint32_t mipLevel,
			shared_ptr<CommandBuffer> cmdBuf);
		VkImageView createImageView(VkImage image, VkImageViewType viewType, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t layoutCount = 1);

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