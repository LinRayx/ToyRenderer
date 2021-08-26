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
			VkImage textureImage;
			VkImageView textureImageView;
			VkDeviceMemory textureImageMemory;
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			VkSampler textureSampler;
		};

	public:
		Texture(shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr);
		void CreateTexture(std::string path, std::string texName);
		std::map< std::string, TextureData > nameToTex;
	private:
		void createTextureImage(std::string path, TextureData& texData);
		void createTextureImageView(TextureData& texData);
		void createTextureSampler(TextureData& texData);

		shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr;

	};

	extern Texture* textureManager;
	void InitTextureMgr( shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr);

	void DestroyTextureMgr();
}

#endif // !DRAW_TEXTURE_H

