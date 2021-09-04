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
	
	void Texture::CreateDepthResource(string name)
	{
		cout << "CreateDepthResource: " << name << endl;
		TextureData texData;
		texData.texWidth = Graphics::Vulkan::getInstance()->GetSwapchain().extent.width;
		texData.texHeight = Graphics::Vulkan::getInstance()->GetSwapchain().extent.height;
		texData.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
		Graphics::Image::getInstance()->createImage(texData.texWidth, texData.texHeight, 1, VK_SAMPLE_COUNT_1_BIT, texData.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texData.textureImage, texData.textureImageMemory);
		createTextureImageView(texData, VK_IMAGE_ASPECT_DEPTH_BIT);
		nameToTex[name] = std::move(texData);
	}


	void Texture::CreateResource(string name, VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags usage)
	{
		cout << "CreateResource: " << name << endl;
		TextureData texData;
		texData.texWidth = width;
		texData.texHeight = height;
		texData.format = format;
		Graphics::Image::getInstance()->createImage(width, height, 1, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texData.textureImage, texData.textureImageMemory);
		createTextureImageView(texData);
		createTextureSampler(texData);
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
		textureManager->CreateTexture("../assets/Metal_Texture/Textures/Metal_Panels_009_basecolor.jpg", "mitsuba-sphere.obj_AlbedoMap");
		textureManager->CreateTexture("../assets/Metal_Texture/Textures/Metal_Panels_009_metallic.jpg", "mitsuba-sphere.obj_MetallicMap");
		textureManager->CreateTexture("../assets/Metal_Texture/Textures/Metal_Panels_009_normal.jpg", "mitsuba-sphere.obj_NormalMap");
		textureManager->CreateTexture("../assets/Metal_Texture/Textures/Metal_Panels_009_roughness.jpg", "mitsuba-sphere.obj_RoughnessMap");
		textureManager->CreateTexture("../assets/Metal_Texture/Textures/Metal_Panels_009_ambientOcclusion.jpg", "mitsuba-sphere.obj_AoMap");

		textureManager->CreateTexture("../assets/Wood/Wood070_1K_Color.jpg", "plane.gltf_AlbedoMap");
		textureManager->CreateTexture("../assets/Wood/Wood070_1K_NormalGL.jpg", "plane.gltf_NormalMap");
		textureManager->CreateTexture("../assets/Wood/Wood070_1K_Roughness.jpg", "plane.gltf_RoughnessMap");

		using namespace Graphics;

		textureManager->CreateDepthResource("depth");
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

		textureManager->CreateDepthResource("GBuffer_depth");

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


	}

	void DestroyTextureMgr()
	{
		delete textureManager;
	}

}