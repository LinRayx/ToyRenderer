#include "Drawable/Material.h"

namespace Draw
{
	Material::Material(std::shared_ptr<Graphics::Vulkan> vulkan_ptr, std::shared_ptr<Graphics::DescriptorPool> desc_pool_ptr)
		: vulkan_ptr(vulkan_ptr)
	{
		desc_ptr = std::make_shared<Graphics::DescriptorSetCore>(vulkan_ptr, desc_pool_ptr);
	}

	void Material::AddLayout(std::string key, Dcb::RawLayout&& layout,
		Graphics::LayoutType layoutType, Graphics::DescriptorType descType, Graphics::StageFlag stage)
	{
		bufs[key] = make_shared<Dcb::Buffer>(std::move(layout));
		buffer_ptrs[key] = make_shared<Graphics::Buffer>(vulkan_ptr, Graphics::BufferUsage::UNIFORM, bufs[key]->GetSizeInBytes());
		desc_ptr->Add(layoutType, descType, stage, buffer_ptrs[key]);
	}

	void Material::AddTexture(Graphics::LayoutType layout_type, Graphics::StageFlag stage, VkImageView textureImageView, VkSampler textureSampler)
	{
		desc_ptr->Add(layout_type, Graphics::DescriptorType::TEXTURE2D, stage, textureImageView, textureSampler);
	}

	void Material::Update(int cur)
	{
		for (auto it : buffer_ptrs)
		{
			it.second->UpdateData(cur, bufs[it.first]->GetSizeInBytes(), bufs[it.first]->GetData());
		}
	}

	void Material::Compile(shared_ptr<Graphics::DescriptorSetLayout> desc_layout_ptr)
	{
		desc_ptr->Compile(desc_layout_ptr);
	}

}