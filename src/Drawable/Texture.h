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
		void CreateCubeTexture(vector<string> paths, std::string texName);
		void CreateCubeTextureWithMipmap(vector<string> paths, std::string texName);
		void CreateResource(string name, VkFormat format, uint32_t dim, VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		void CreateCubeResource(string name, VkFormat format, uint32_t dim, uint32_t mipLevels = 1);
		void CreateDepthResource(string name);
		std::map< std::string, TextureData > nameToTex;
	private:
		void createTextureImage(std::string path, TextureData& texData);
		void createTextureImageView(TextureData& texData, VkImageAspectFlagBits flag = VK_IMAGE_ASPECT_COLOR_BIT, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, uint32_t layoutCount = 1);
		void createCubeTextureImageView(TextureData& texData, uint32_t layoutCount = 6, uint32_t mipLevel = 1);
		void createTextureSampler(TextureData& texData, uint32_t mipLevel = 1, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);

		shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr;
	};

	extern Texture* textureManager;
	void InitTextureMgr( shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr);

	void DestroyTextureMgr();
}

#endif // !DRAW_TEXTURE_H

