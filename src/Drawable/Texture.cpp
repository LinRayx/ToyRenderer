#include "Drawable/Texture.h"
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Draw
{
	Texture::Texture( shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr) :  cmdBuf_ptr(cmdBuf_ptr)
	{

	}

	Texture::~Texture()
	{
		for (auto& tex : nameToTex) {
			vkDestroyImage(Graphics::Vulkan::getInstance()->GetDevice().device, tex.second.textureImage, Graphics::Vulkan::getInstance()->GetDevice().allocator);
			vkDestroyImageView(Graphics::Vulkan::getInstance()->GetDevice().device, tex.second.textureImageView, Graphics::Vulkan::getInstance()->GetDevice().allocator);
			if (tex.second.textureSampler != VK_NULL_HANDLE)
				vkDestroySampler(Graphics::Vulkan::getInstance()->GetDevice().device, tex.second.textureSampler, Graphics::Vulkan::getInstance()->GetDevice().allocator);
		}
	}

	void Texture::CreateTexture(std::string path, std::string texName)
	{
		if (nameToTex.count(texName) > 0)
			return;
		TextureData texData;
		texData.format = VK_FORMAT_R8G8B8A8_UNORM;
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
			if (textures[i] == nullptr) {
				throw std::runtime_error("can not read image: " + paths[i]);
			}
		}

		VkDeviceSize imageSize = data.texWidth * data.texHeight * 4 * paths.size();
		Graphics::Buffer::CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			data.stagingBuffer, data.stagingBufferMemory);
		void* bufferData;

		vkMapMemory(Graphics::Vulkan::getInstance()->GetDevice().device, data.stagingBufferMemory, 0, imageSize, 0, &bufferData);
		size_t offset = 0;
		size_t sz = data.texWidth * data.texHeight * 4;
		for (size_t i = 0; i < textures.size(); ++i) {
			memcpy((char*)bufferData + offset, textures[i], sz);
			offset += sz;
		}
		vkUnmapMemory(Graphics::Vulkan::getInstance()->GetDevice().device, data.stagingBufferMemory);

		for (size_t i = 0; i < textures.size(); ++i) {
			stbi_image_free(textures[i]);
		}
		// SRGB的话会自动做gamma矫正
		data.format = VK_FORMAT_R8G8B8A8_UNORM;

		Graphics::Image::getInstance()->createCubeImage(data.texWidth, data.texHeight, 1, VK_SAMPLE_COUNT_1_BIT, data.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, data.textureImage, data.textureImageMemory);
		cmdBuf_ptr->transitionImageLayout(data.textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6);
		cmdBuf_ptr->copyBufferToCubeImage(data.stagingBuffer, data.textureImage, static_cast<uint32_t>(data.texWidth), static_cast<uint32_t>(data.texHeight), 6);
		cmdBuf_ptr->transitionImageLayout(data.textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6);
		vkDestroyBuffer(Graphics::Vulkan::getInstance()->GetDevice().device, data.stagingBuffer, nullptr);
		vkFreeMemory(Graphics::Vulkan::getInstance()->GetDevice().device, data.stagingBufferMemory, nullptr);

	

		createCubeTextureImageView(data);
		createTextureSampler(data);

		nameToTex[texName] = std::move(data);
	}

	void Texture::CreateCubeTextureWithMipmap(vector<string> paths, std::string texName)
	{
		vector<stbi_uc*> textures(paths.size());
		
		TextureData data;

		for (size_t i = 0; i < paths.size(); ++i) {
			textures[i] = stbi_load(paths[i].c_str(), &data.texWidth, &data.texHeight, &data.texChannel, STBI_rgb_alpha);
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

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		auto cmd = cmdBuf_ptr->beginSingleTimeCommands();
		cmdBuf_ptr->insertImageMemoryBarrier(
			cmd,
			data.textureImage,
			0,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			subresourceRange);

		cmdBuf_ptr->copyBufferToCubeImage(data.stagingBuffer, data.textureImage, static_cast<uint32_t>(data.texWidth), static_cast<uint32_t>(data.texHeight), 6);

		cmdBuf_ptr->insertImageMemoryBarrier(
			cmd,
			data.textureImage,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_TRANSFER_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			subresourceRange);
		cmdBuf_ptr->endSingleTimeCommands(cmd);
		

		vkDestroyBuffer(Graphics::Vulkan::getInstance()->GetDevice().device, data.stagingBuffer, nullptr);
		vkFreeMemory(Graphics::Vulkan::getInstance()->GetDevice().device, data.stagingBufferMemory, nullptr);

		uint32_t mipLevel = floor(log2(std::max(data.texWidth, data.texHeight))) + 1;

		Graphics::Image::getInstance()->generateMipmap(data.textureImage, data.texWidth, data.texHeight, mipLevel, cmdBuf_ptr);

		data.format = VK_FORMAT_R8G8B8A8_SRGB;

		createCubeTextureImageView(data, 6, mipLevel);
		createTextureSampler(data, mipLevel, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT);

		nameToTex[texName] = std::move(data);
	}

	void Texture::CreateResource(string name, VkFormat format, uint32_t dim, VkImageUsageFlags usage)
	{
		cout << "CreateResource: " << name << endl;
		TextureData texData;
		texData.texWidth = texData.texHeight = dim;
		texData.format = format;
		Graphics::Image::getInstance()->createImage(dim, dim, 1, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texData.textureImage, texData.textureImageMemory);
		createTextureImageView(texData, VK_IMAGE_ASPECT_COLOR_BIT, format);
		createTextureSampler(texData, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		nameToTex[name] = std::move(texData);
	}

	void Texture::CreateCubeResource(string name, VkFormat format, uint32_t dim, uint32_t mipLevels)
	{
		cout << "CreateCubeResource: " << name << endl;
		TextureData texData;
		texData.texWidth = texData.texHeight = dim;
		texData.format = format;
		Graphics::Image::getInstance()->createCubeImage(dim, dim, mipLevels, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texData.textureImage, texData.textureImageMemory);
		createCubeTextureImageView(texData, 6, mipLevels);
		createTextureSampler(texData, mipLevels);
		nameToTex[name] = std::move(texData);
	}
	
	void Texture::CreateDepthResource(string name)
	{
		cout << "CreateDepthResource: " << name << endl;
		TextureData texData;
		texData.texWidth = Graphics::Vulkan::getInstance()->GetSwapchain().extent.width;
		texData.texHeight = Graphics::Vulkan::getInstance()->GetSwapchain().extent.height;
		texData.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
		Graphics::Image::getInstance()->createImage(texData.texWidth, texData.texHeight, 1, VK_SAMPLE_COUNT_1_BIT, texData.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texData.textureImage, texData.textureImageMemory);
		createTextureImageView(texData, VK_IMAGE_ASPECT_DEPTH_BIT, VK_FORMAT_D32_SFLOAT_S8_UINT);
		nameToTex[name] = std::move(texData);
	}

	void Texture::createTextureImage(std::string path, TextureData& texData)
	{
		cout << "createTextureImage: " << path.c_str() << endl;
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

		Graphics::Image::getInstance()->createImage(texWidth, texHeight, 1, VK_SAMPLE_COUNT_1_BIT, texData.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
		cmdBuf_ptr->transitionImageLayout(textureImage,  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		cmdBuf_ptr->copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		cmdBuf_ptr->transitionImageLayout(textureImage,  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		vkDestroyBuffer(Graphics::Vulkan::getInstance()->GetDevice().device, stagingBuffer, nullptr);
		vkFreeMemory(Graphics::Vulkan::getInstance()->GetDevice().device, stagingBufferMemory, nullptr);
	}

	void Texture::createTextureImageView(TextureData& texData, VkImageAspectFlagBits flag, VkFormat format, uint32_t layoutCount)
	{
		texData.textureImageView = Graphics::Image::getInstance()->createImageView(texData.textureImage, VK_IMAGE_VIEW_TYPE_2D, texData.format, flag, 1, layoutCount);
	}

	void Texture::createCubeTextureImageView(TextureData& texData, uint32_t layoutCount, uint32_t mipLevel)
	{
		texData.textureImageView = Graphics::Image::getInstance()->createImageView(texData.textureImage, VK_IMAGE_VIEW_TYPE_CUBE, texData.format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevel, layoutCount);
	}

	void Texture::createTextureSampler(TextureData& texData, uint32_t mipLevel, VkSamplerAddressMode addressMode)
	{
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(Graphics::Vulkan::getInstance()->GetDevice().physical_device, &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = addressMode;
        samplerInfo.addressModeV = addressMode;
        samplerInfo.addressModeW = addressMode;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(mipLevel);
        samplerInfo.mipLodBias = 0.0f;

        if (vkCreateSampler(Graphics::Vulkan::getInstance()->GetDevice().device, &samplerInfo, nullptr, &texData.textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
	}

	Texture* textureManager;

	void InitTextureMgr(shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr)
	{
		// +X，-X，+Y，-Y，+Z，-Z
		//		+Y 
		//	-X	+Z	+X	-Z
		//		-Y
		textureManager = new Texture(cmdBuf_ptr);
		vector<string> paths;
		paths.emplace_back("../assets/skybox/skybox2/px.png");
		paths.emplace_back("../assets/skybox/skybox2/nx.png");
		paths.emplace_back("../assets/skybox/skybox2/py.png");
		paths.emplace_back("../assets/skybox/skybox2/ny.png");
		paths.emplace_back("../assets/skybox/skybox2/pz.png");
		paths.emplace_back("../assets/skybox/skybox2/nz.png");

		textureManager->CreateCubeTexture(std::move(paths), "skybox_texture");
		textureManager->CreateTexture("../assets/Cerberus_by_Andrew_Maximov/Textures/Cerberus_A.tga", "albedoMap");
		textureManager->CreateTexture("../assets/Cerberus_by_Andrew_Maximov/Textures/Cerberus_M.tga", "metallicMap");
		textureManager->CreateTexture("../assets/Cerberus_by_Andrew_Maximov/Textures/Cerberus_N.tga", "normalMap");
		textureManager->CreateTexture("../assets/Cerberus_by_Andrew_Maximov/Textures/Cerberus_R.tga", "roughnessMap");
		textureManager->CreateTexture("../assets/Cerberus_by_Andrew_Maximov/Textures/Raw/Cerberus_AO.tga", "aoMap");
	}

	void DestroyTextureMgr()
	{
		delete textureManager;
	}

}