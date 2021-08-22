#ifndef MATERIAL_H
#define MATERIAL_H
#include "VulkanCore/Vulkan.h"
#include "VulkanCore/DescriptorPool.h"
#include "VulkanCore/DescriptorSet.h"
#include <map>
#include "DynamicVariable/DynamicConstant.h"

namespace Draw
{
	class Material
	{
	public:
		Material(std::shared_ptr<Graphics::Vulkan> vulkan_ptr, std::shared_ptr<Graphics::DescriptorPool> desc_pool_ptr);
		void AddLayout(std::string key, Dcb::RawLayout&& layout,
			Graphics::LayoutType layoutType, Graphics::DescriptorType descType, Graphics::StageFlag stage);

		void AddTexture(Graphics::LayoutType layout_type, Graphics::StageFlag stage, VkImageView textureImageView, VkSampler textureSampler);

		template<typename T>
		void Update(std::string key1, std::string key2, T value)
		{
			(*bufs[key1])[key2] = value;
		}

		void Update(int cur);
		
		void Compile(shared_ptr<Graphics::DescriptorSetLayout> desc_layout_ptr);

	public:
		std::shared_ptr<Graphics::DescriptorSetCore> desc_ptr;

		std::shared_ptr<Graphics::Vulkan> vulkan_ptr;
		std::map<std::string, shared_ptr<Graphics::Buffer>> buffer_ptrs;
		std::map<std::string, shared_ptr<Dcb::Buffer>> bufs;
	};
}

#endif // !MATERIAL_H
