#include "Drawable/Texture.h"
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Draw
{
	Texture::Texture( shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr) :  cmdBuf_ptr(cmdBuf_ptr)
	{

	}

	void Texture::CreateTexture(std::string path, std::string texName)
	{
		if (nameToTex.count(texName) > 0)
			return;
		TextureData texData;
		createTextureImage(path, texData);
		createTextureImageView(texData);
		createTextureSampler(texData);
		nameToTex[texName] = std::move(texData);
	}

	void Texture::CreateCubeTexture(vector<string> paths, std::string texName)
	{
		vector<stbi_uc*> textures(paths.size());
		
		TextureData data;

		for (size_t i = 0; i < paths.size(); ++i) {
			textures[i] = stbi_load(paths[i].c_str(), &data.texWidth, &data.texHeight, &data.texChannel, STBI_rgb_alpha);
			cout << sizeof(*textures[i]) << endl;
		}

		VkDeviceSize imageSize = data.texWidth * data.texHeight * 4 * paths.size();
		Graphics::Buffer::CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			data.stagingBuffer, data.stagingBufferMemory);
		void* bufferData;

		vkMapMemory(Graphics::Vulkan::getInstance()->GetDevice().device, data.stagingBufferMemory, 0, imageSize, 0, &bufferData);
		size_t offset = 0;
		size_t sz = data.texWidth * data.texHeight * 4;
		for (size_t i = 0; i < textures.size(); ++i) {
			memcpy((char*)bufferData+offset, textures[i], sz);
			offset += sz;
		}
		vkUnmapMemory(Graphics::Vulkan::getInstance()->GetDevice().device, data.stagingBufferMemory);

		for (size_t i = 0; i < textures.size(); ++i) {
			stbi_image_free(textures[i]);
		}
		
		Graphics::Image::getInstance()->createCubeImage(data.texWidth, data.texHeight, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, data.textureImage, data.textureImageMemory);
		cmdBuf_ptr->transitionImageLayout(data.textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6);
		cmdBuf_ptr->copyBufferToCubeImage(data.stagingBuffer, data.textureImage, static_cast<uint32_t>(data.texWidth), static_cast<uint32_t>(data.texHeight), 6);
		cmdBuf_ptr->transitionImageLayout(data.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6);
		vkDestroyBuffer(Graphics::Vulkan::getInstance()->GetDevice().device, data.stagingBuffer, nullptr);
		vkFreeMemory(Graphics::Vulkan::getInstance()->GetDevice().device, data.stagingBufferMemory, nullptr);

		createCubeTextureImageView(data);
		createTextureSampler(data);

		nameToTex[texName] = std::move(data);
	}

	void Texture::createTextureImage(std::string path, TextureData& texData)
	{
		auto& texWidth = texData.texWidth;
		auto& texHeight = texData.texHeight;
		auto& texChannel = texData.texChannel;
		auto& stagingBuffer = texData.stagingBuffer;
		auto& stagingBufferMemory = texData.stagingBufferMemory;
		auto& textureImage = texData.textureImage;
		auto& textureImageMemory = texData.textureImageMemory;
		
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannel, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image: " + path);
		}


		Graphics::Buffer::CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);
		void* data;

		vkMapMemory(Graphics::Vulkan::getInstance()->GetDevice().device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(Graphics::Vulkan::getInstance()->GetDevice().device, stagingBufferMemory);

		stbi_image_free(pixels);

		Graphics::Image::getInstance()->createImage(texWidth, texHeight, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
		cmdBuf_ptr->transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		cmdBuf_ptr->copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		cmdBuf_ptr->transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		vkDestroyBuffer(Graphics::Vulkan::getInstance()->GetDevice().device, stagingBuffer, nullptr);
		vkFreeMemory(Graphics::Vulkan::getInstance()->GetDevice().device, stagingBufferMemory, nullptr);
	}

	void Texture::createTextureImageView(TextureData& texData, uint32_t layoutCount)
	{
		texData.textureImageView = Graphics::Image::getInstance()->createImageView(texData.textureImage, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1, layoutCount);
	}

	void Texture::createCubeTextureImageView(TextureData& texData, uint32_t layoutCount)
	{
		texData.textureImageView = Graphics::Image::getInstance()->createImageView(texData.textureImage, VK_IMAGE_VIEW_TYPE_CUBE, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1, layoutCount);
	}

	void Texture::createTextureSampler(TextureData& texData)
	{
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(Graphics::Vulkan::getInstance()->GetDevice().physical_device, &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(1);
        samplerInfo.mipLodBias = 0.0f;

        if (vkCreateSampler(Graphics::Vulkan::getInstance()->GetDevice().device, &samplerInfo, nullptr, &texData.textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
	}

	Texture* textureManager;

	void InitTextureMgr(shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr)
	{
		textureManager = new Texture(cmdBuf_ptr);
		vector<string> paths;
		paths.emplace_back("../assets/skybox/right1.jpg");
		paths.emplace_back("../assets/skybox/left1.jpg");
		paths.emplace_back("../assets/skybox/top1.jpg");
		paths.emplace_back("../assets/skybox/bottom1.jpg");
		paths.emplace_back("../assets/skybox/front1.jpg");
		paths.emplace_back("../assets/skybox/back1.jpg");

		textureManager->CreateCubeTexture(std::move(paths), "skybox_texture");
	}

	void DestroyTextureMgr()
	{
		delete textureManager;
	}

}