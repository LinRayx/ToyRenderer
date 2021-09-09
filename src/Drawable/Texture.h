#ifndef DRAW_TEXTURE_H
#define DRAW_TEXTURE_H

#include <string>
#include <map>
#include "VulkanCore/vulkan_core_headers.h"
#include <memory>

namespace Draw
{
	class Texture
	{
	private:
		struct TextureData {
			int texWidth;
			int texHeight;
			int texChannel;
			VkFormat format;
			VkImage textureImage;
			VkImageView textureImageView;
			VkDeviceMemory textureImageMemory;
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			VkSampler textureSampler = VK_NULL_HANDLE;
		};

	public:
		Texture(shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr);
		~Texture();
		void CreateTexture(std::string path, std::string texName);
		void CreateTextureFromData(string texName, void* buffer, VkDeviceSize bufferSize, VkFormat format, uint32_t texWidth, uint32_t texHeight, VkFilter filter = VK_FILTER_LINEAR, VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		void CreateCubeTexture(vector<string> paths, std::string texName);
		void CreateCubeTextureWithMipmap(vector<string> paths, std::string texName);
		void CreateResource(string name, VkFormat format, uint32_t dim, VkImageUsageFlags usage);
		void CreateResource(string name, VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags usage, bool flag = false);
		void CreateCubeResource(string name, VkFormat format, uint32_t dim, uint32_t mipLevels = 1);
		void CreateCubeResource(string name, VkImageCreateInfo imageCreateInfo, VkSamplerCreateInfo sampler, VkImageViewCreateInfo view);
		void CreateDepthResource(string name, int width, int height, bool flag = false);
		std::map< std::string, TextureData > nameToTex;
	private:
		void createTextureImage(std::string path, TextureData& texData);
		void createTextureImageView(TextureData& texData, VkImageAspectFlagBits flag = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t layoutCount = 1);
		void createCubeTextureImageView(TextureData& texData, uint32_t layoutCount = 6, uint32_t mipLevel = 1);
		void createTextureSampler(TextureData& texData, uint32_t mipLevel = 1, VkFilter filter = VK_FILTER_LINEAR, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);

		shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr;
	};

	extern Texture* textureManager;
	void InitTextureMgr( shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr);

	void DestroyTextureMgr();
}

#endif // !DRAW_TEXTURE_H

