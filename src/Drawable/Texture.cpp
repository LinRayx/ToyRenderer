#include "Drawable/Texture.h"
#include <stdexcept>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <tinygltf/tiny_gltf.h>

namespace Draw
{
	Texture::Texture( shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr) :  cmdBuf_ptr(cmdBuf_ptr)
	{
		// stbi_set_flip_vertically_on_load(true);
	}

	Texture::~Texture()
	{
		for (auto& tex : nameToTex) {
			if (tex.second.textureImage != VK_NULL_HANDLE)
				vkDestroyImage(Graphics::Vulkan::getInstance()->GetDevice().device, tex.second.textureImage, Graphics::Vulkan::getInstance()->GetDevice().allocator);
			if (tex.second.textureImageView != VK_NULL_HANDLE)
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

	void Texture::CreateTextureFromData(string texName, void* buffer, VkDeviceSize bufferSize, VkFormat format, uint32_t texWidth, uint32_t texHeight, VkFilter filter, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout)
	{
		cout << "Texture::CreateTextureFromData: " << texName << endl;
		TextureData texData;
		texData.format = format;
		texData.texWidth = texWidth;
		texData.texHeight = texHeight;

		Graphics::Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			texData.stagingBuffer, texData.stagingBufferMemory);

		void* data;

		vkMapMemory(Graphics::Vulkan::getInstance()->GetDevice().device, texData.stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, buffer, static_cast<size_t>(bufferSize));
		vkUnmapMemory(Graphics::Vulkan::getInstance()->GetDevice().device, texData.stagingBufferMemory);

		Graphics::Image::getInstance()->createImage(texWidth, texHeight, 1, VK_SAMPLE_COUNT_1_BIT, texData.format, VK_IMAGE_TILING_OPTIMAL, imageUsageFlags | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texData.textureImage, texData.textureImageMemory);
		cmdBuf_ptr->transitionImageLayout(texData.textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		cmdBuf_ptr->copyBufferToImage(texData.stagingBuffer, texData.textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		cmdBuf_ptr->transitionImageLayout(texData.textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageLayout);
		vkDestroyBuffer(Graphics::Vulkan::getInstance()->GetDevice().device, texData.stagingBuffer, nullptr);
		vkFreeMemory(Graphics::Vulkan::getInstance()->GetDevice().device, texData.stagingBufferMemory, nullptr);

		createTextureSampler(texData, 1, filter);
		createTextureImageView(texData);
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

	void Texture::CreateCSMDepthAndViews(string depthName, string cascadeName, VkFormat format,int width, int height, int count)
	{
		TextureData texData;
		texData.format = format;
		VkImageCreateInfo imageInfo = Graphics::initializers::imageCreateInfo();
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = count;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.format = format;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		Graphics::Image::getInstance()->createImage(imageInfo, texData.textureImage, texData.textureImageMemory);

		// Full depth map view (all layers)
		VkImageViewCreateInfo viewInfo = Graphics::initializers::imageViewCreateInfo();
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		viewInfo.format = format;
		viewInfo.subresourceRange = {};
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = count;
		viewInfo.image = texData.textureImage;
		texData.textureImageView = Graphics::Image::getInstance()->createImageView(viewInfo);

		// Shared sampler for cascade depth reads
		VkSamplerCreateInfo sampler = Graphics::initializers::samplerCreateInfo();
		sampler.magFilter = VK_FILTER_LINEAR;
		sampler.minFilter = VK_FILTER_LINEAR;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.addressModeV = sampler.addressModeU;
		sampler.addressModeW = sampler.addressModeU;
		sampler.mipLodBias = 0.0f;
		sampler.maxAnisotropy = 1.0f;
		sampler.minLod = 0.0f;
		sampler.maxLod = 1.0f;
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		vkCreateSampler(Graphics::Vulkan::getInstance()->GetDevice().device, &sampler, nullptr, &texData.textureSampler);

		for (int i = 0; i < count; ++i) {
			VkImageViewCreateInfo viewInfo = Graphics::initializers::imageViewCreateInfo();
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			viewInfo.format = format;
			viewInfo.subresourceRange = {};
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = i;
			viewInfo.subresourceRange.layerCount = 1;
			viewInfo.image = texData.textureImage;

			TextureData cas;
			cas.textureImageView = Graphics::Image::getInstance()->createImageView(viewInfo);
			cas.format = format;
			nameToTex[cascadeName + to_string(i)] = std::move(cas);
		}
		nameToTex[depthName] = std::move(texData);
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
		Graphics::Image::getInstance()->insertImageMemoryBarrier(
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

		Graphics::Image::getInstance()->insertImageMemoryBarrier(
			cmd,
			data.textureImage,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_TRANSFER_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT, // 等待该阶段所有operation执行完后才开始layout 转换
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			subresourceRange);
		cmdBuf_ptr->endSingleTimeCommands(cmd);
		

		vkDestroyBuffer(Graphics::Vulkan::getInstance()->GetDevice().device, data.stagingBuffer, nullptr);
		vkFreeMemory(Graphics::Vulkan::getInstance()->GetDevice().device, data.stagingBufferMemory, nullptr);

		uint32_t mipLevel = floor(log2(std::max(data.texWidth, data.texHeight))) + 1;

		Graphics::Image::getInstance()->generateMipmap(data.textureImage, data.texWidth, data.texHeight, mipLevel, cmdBuf_ptr);

		data.format = VK_FORMAT_R8G8B8A8_SRGB;

		createCubeTextureImageView(data, 6, mipLevel);
		createTextureSampler(data, mipLevel);

		nameToTex[texName] = std::move(data);
	}

	void Texture::CreateResource(string name, VkFormat format, uint32_t dim, VkImageUsageFlags usage)
	{
		cout << "CreateResource: " << name << endl;
		CreateResource(name, format, dim, dim, usage);
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
	
	void Texture::CreateDepthResource(string name, int width, int height, bool flag)
	{
		cout << "CreateDepthResource: " << name << endl;
		TextureData texData;
		texData.texWidth = width;
		texData.texHeight = height;
		texData.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
		Graphics::Image::getInstance()->createImage(texData.texWidth, texData.texHeight, 1, VK_SAMPLE_COUNT_1_BIT, texData.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texData.textureImage, texData.textureImageMemory);
		createTextureImageView(texData, VK_IMAGE_ASPECT_DEPTH_BIT);
		if (flag)
		{
			auto cmd = cmdBuf_ptr->beginSingleTimeCommands();
			Graphics::Image::getInstance()->setImageLayout(
				cmd,
				texData.textureImage,
				VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			);
			cmdBuf_ptr->endSingleTimeCommands(cmd);
		}
		nameToTex[name] = std::move(texData);
	}


	void Texture::CreateResource(string name, VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags usage, bool flag)
	{
		cout << "CreateResource: " << name << endl;
		TextureData texData;
		texData.texWidth = width;
		texData.texHeight = height;
		texData.format = format;
		Graphics::Image::getInstance()->createImage(width, height, 1, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texData.textureImage, texData.textureImageMemory);
		createTextureImageView(texData);
		createTextureSampler(texData);
		if (flag)
		{
			auto cmd = cmdBuf_ptr->beginSingleTimeCommands();
			Graphics::Image::getInstance()->setImageLayout(
				cmd,
				texData.textureImage,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			);
			cmdBuf_ptr->endSingleTimeCommands(cmd);
		}
		nameToTex[name] = std::move(texData);
	}

	void Texture::CreateCubeResource(string name, VkImageCreateInfo imageCreateInfo, VkSamplerCreateInfo sampler, VkImageViewCreateInfo view)
	{
		cout << "CreateCubeResource: " << name << endl;
		TextureData texData;
		texData.texWidth = imageCreateInfo.extent.width;
		texData.texHeight = imageCreateInfo.extent.height;
		texData.format = imageCreateInfo.format;
		Graphics::Image::getInstance()->createImage(imageCreateInfo, texData.textureImage, texData.textureImageMemory);
		view.image = texData.textureImage;
		texData.textureImageView = Graphics::Image::getInstance()->createImageView(view);
		texData.textureSampler = Graphics::Image::getInstance()->createSampler(sampler);

		// Image barrier for optimal image (target)
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 6;
		{
			auto cmd = cmdBuf_ptr->beginSingleTimeCommands();
			Graphics::Image::getInstance()->setImageLayout(
				cmd,
				texData.textureImage,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				subresourceRange
			);
			cmdBuf_ptr->endSingleTimeCommands(cmd);
		}

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

	void Texture::createTextureImageView(TextureData& texData, VkImageAspectFlagBits flag, uint32_t layoutCount)
	{
		texData.textureImageView = Graphics::Image::getInstance()->createImageView(texData.textureImage, VK_IMAGE_VIEW_TYPE_2D, texData.format, flag, 1, layoutCount);
	}

	void Texture::createCubeTextureImageView(TextureData& texData, uint32_t layoutCount, uint32_t mipLevel)
	{
		texData.textureImageView = Graphics::Image::getInstance()->createImageView(texData.textureImage, VK_IMAGE_VIEW_TYPE_CUBE, texData.format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevel, layoutCount);
	}

	void Texture::createTextureSampler(TextureData& texData, uint32_t mipLevel, VkFilter filter,  VkSamplerAddressMode addressMode)
	{
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(Graphics::Vulkan::getInstance()->GetDevice().physical_device, &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = filter;
        samplerInfo.minFilter = filter;
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

	void createCubeDepthResource()
	{
		VkFormat format = VK_FORMAT_R32_SFLOAT;
		uint32_t dim = 1024;

		// Cube map image description
		VkImageCreateInfo imageCreateInfo = Graphics::initializers::imageCreateInfo();
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.extent = { dim, dim, 1 };
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 6;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT ;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		// Create sampler
		VkSamplerCreateInfo sampler = Graphics::initializers::samplerCreateInfo();
		sampler.magFilter = VK_FILTER_LINEAR;
		sampler.minFilter = VK_FILTER_LINEAR;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		sampler.addressModeV = sampler.addressModeU;
		sampler.addressModeW = sampler.addressModeU;
		sampler.mipLodBias = 0.0f;
		sampler.maxAnisotropy = 1.0f;
		sampler.compareOp = VK_COMPARE_OP_NEVER;
		sampler.minLod = 0.0f;
		sampler.maxLod = 1.0f;
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		// Create image view
		VkImageViewCreateInfo view = Graphics::initializers::imageViewCreateInfo();
		view.image = VK_NULL_HANDLE;
		view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		view.format = format;
		view.components = { VK_COMPONENT_SWIZZLE_R };
		view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		view.subresourceRange.layerCount = 6;
		textureManager->CreateCubeResource("omni_depth_map", std::move(imageCreateInfo), std::move(sampler), std::move(view));
		textureManager->CreateResource("omni_color_attachment", VK_FORMAT_R32_SFLOAT, dim, dim, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true);

		textureManager->CreateDepthResource("omni_depth_attachment", dim, dim, true);
	}

	void InitTextureMgr(shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr)
	{
		// +X，-X，+Y，-Y，+Z，-Z
		//		+Y 
		//	-X	+Z	+X	-Z
		//		-Y
		textureManager = new Texture(cmdBuf_ptr);
		vector<string> paths;
		paths.emplace_back("../assets/newport_loft/px.png");
		paths.emplace_back("../assets/newport_loft/nx.png");
		paths.emplace_back("../assets/newport_loft/py.png");
		paths.emplace_back("../assets/newport_loft/ny.png");
		paths.emplace_back("../assets/newport_loft/pz.png");
		paths.emplace_back("../assets/newport_loft/nz.png");

		textureManager->CreateCubeTexture(std::move(paths), "skybox_texture");
		
		//textureManager->CreateTexture("../assets/gold/albedo.png", "mitsuba-sphere.obj_AlbedoMap");
		//textureManager->CreateTexture("../assets/gold/metallic.png", "mitsuba-sphere.obj_MetallicMap");
		//textureManager->CreateTexture("../assets/gold/normal.png", "mitsuba-sphere.obj_NormalMap");
		//textureManager->CreateTexture("../assets/gold/roughness.png", "mitsuba-sphere.obj_RoughnessMap");

		textureManager->CreateTexture("../assets/MC003_Kozakura_Mari.png", "mary.gltf_Albedo");


		textureManager->CreateTexture("../assets/StoneWall/Textures/wall-stonework-sheen_albedo.png", "luxball.gltf_Albedo");
		textureManager->CreateTexture("../assets/StoneWall/Textures/wall-stonework-sheen_metallic.png", "luxball.gltf_Metallic");
		textureManager->CreateTexture("../assets/StoneWall/Textures/wall-stonework-sheen_normal-ogl.png", "luxball.gltf_Normal");
		textureManager->CreateTexture("../assets/StoneWall/Textures/wall-stonework-sheen_roughness.png", "luxball.gltf_Roughness");

		//textureManager->CreateTexture("../assets/rustediron/Textures/rustediron2_basecolor.png", "mitsuba-sphere.obj_AlbedoMap");
		//textureManager->CreateTexture("../assets/rustediron/Textures/rustediron2_metallic.png", "mitsuba-sphere.obj_MetallicMap");
		//textureManager->CreateTexture("../assets/rustediron/Textures/rustediron2_normal.png", "mitsuba-sphere.obj_NormalMap");
		//textureManager->CreateTexture("../assets/rustediron/Textures/rustediron2_roughness.png", "mitsuba-sphere.obj_RoughnessMap");

		textureManager->CreateTexture("../assets/Wood/Wood070_1K_Color.jpg", "plane.gltf_Albedo");
		textureManager->CreateTexture("../assets/Wood/Wood070_1K_NormalGL.jpg", "plane.gltf_Normal");
		textureManager->CreateTexture("../assets/Wood/Wood070_1K_Roughness.jpg", "plane.gltf_Roughness");

		using namespace Graphics;

		textureManager->CreateDepthResource("depth", Vulkan::getInstance()->GetWidth(), Vulkan::getInstance()->GetHeight());
		textureManager->CreateResource("brdf_lut", VK_FORMAT_R16G16_SFLOAT, 512,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		textureManager->CreateResource("irradiance_attachment", VK_FORMAT_R32G32B32A32_SFLOAT, 64,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		textureManager->CreateCubeResource("irradiance_map", VK_FORMAT_R32G32B32A32_SFLOAT, 64);
		// VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
		uint32_t numMips = static_cast<uint32_t>(floor(log2(512))) + 1;
		textureManager->CreateResource("prefilter_attachment", VK_FORMAT_R16G16B16A16_SFLOAT, 512,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		textureManager->CreateCubeResource("prefilter_map", VK_FORMAT_R16G16B16A16_SFLOAT, 512, numMips);

		// deferred resouces
		textureManager->CreateResource("GBuffer_position", VK_FORMAT_R32G32B32A32_SFLOAT,
			Vulkan::getInstance()->GetWidth(),
			Vulkan::getInstance()->GetHeight(),
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		textureManager->CreateResource("GBuffer_normals", VK_FORMAT_R8G8B8A8_UNORM,
			Vulkan::getInstance()->GetWidth(),
			Vulkan::getInstance()->GetHeight(),
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		textureManager->CreateResource("GBuffer_albedo", VK_FORMAT_R8G8B8A8_UNORM,
			Vulkan::getInstance()->GetWidth(),
			Vulkan::getInstance()->GetHeight(),
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		textureManager->CreateResource("GBuffer_metallic_roughness", VK_FORMAT_R16G16B16A16_SFLOAT,
			Vulkan::getInstance()->GetWidth(),
			Vulkan::getInstance()->GetHeight(),
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		textureManager->CreateDepthResource("GBuffer_depth", Vulkan::getInstance()->GetWidth(), Vulkan::getInstance()->GetHeight());

		// SSAO
		textureManager->CreateTextureFromData(
			"ssaoNoiseMap",
			Gloable::SSAO::ssaoNoise.data(),
			Gloable::SSAO::ssaoNoise.size() * sizeof(glm::vec4),
			VK_FORMAT_R32G32B32A32_SFLOAT,
			Gloable::SSAO::SSAO_NOISE_DIM,
			Gloable::SSAO::SSAO_NOISE_DIM,
			VK_FILTER_NEAREST);

		textureManager->CreateResource("ssaoMap",
			VK_FORMAT_R8_UNORM, Vulkan::getInstance()->GetWidth(), Vulkan::getInstance()->GetHeight(),
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		textureManager->CreateResource("ssaoBlurMap",
			VK_FORMAT_R8_UNORM, Vulkan::getInstance()->GetWidth(), Vulkan::getInstance()->GetHeight(),
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		textureManager->CreateResource("diffuseMap",
			VK_FORMAT_R32G32B32A32_SFLOAT, Vulkan::getInstance()->GetWidth(), Vulkan::getInstance()->GetHeight(),
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		textureManager->CreateResource("specularMap",
			VK_FORMAT_R32G32B32A32_SFLOAT, Vulkan::getInstance()->GetWidth(), Vulkan::getInstance()->GetHeight(),
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		// Shadow
		createCubeDepthResource();

		textureManager->CreateCSMDepthAndViews("casDepth", "cascades", VK_FORMAT_D32_SFLOAT_S8_UINT, 1024, 1024, SHADOWMAP_COUNT);
	}

	VkImageView GetImageView(string name)
	{
		return textureManager->nameToTex[name].textureImageView;
	}
	VkSampler GetSampler(string name)
	{
		return textureManager->nameToTex[name].textureSampler;
	}
	void DestroyTextureMgr()
	{
		delete textureManager;
	}

}