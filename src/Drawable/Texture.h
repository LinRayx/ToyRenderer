#ifndef DRAW_TEXTURE_H
#define DRAW_TEXTURE_H

#include <string>
#include <map>
#include "VulkanCore/vulkan_core_headers.h"
#include <memory>


namespace Draw
{
	/*
	const是默认具有内部链接属性(internal linkage)，
	也就是说仅在定义这个变量的文件内可见，在链接时对外是不可见的。
	头文件里的全局常量实际上是include该头文件的CPP都有自己的一份额外的定义，
	而对于其他编译单元来说是透明的，不会造成重定义的链接错误。

	而至于改成const char*的写法，就需要注意了：
	这时候定义的只是一个指向常量字符串的普通指针而已，而不是常量指针。正确的写法应该是const char* const
	*/
	const int SHADOWMAP_COUNT = 4;
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

		void CreateCSMDepthAndViews(string depthName, string cascadeName, VkFormat format, int width, int height, int count);
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
	VkImageView GetImageView(string name);
	VkSampler GetSampler(string name);
	void DestroyTextureMgr();
}

#endif // !DRAW_TEXTURE_H

