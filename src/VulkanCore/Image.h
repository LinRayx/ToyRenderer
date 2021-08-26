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
		static Image* getInstance() {
			if (instance == NULL) {
				instance = new Image();
			}
			return instance;
		}



		void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

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