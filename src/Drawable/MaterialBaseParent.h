#ifndef MATERIAL_BASE_PARENT_H
#define MATERIAL_BASE_PARENT_H

#include "VulkanCore/vulkan_core_headers.h"

#include <map>
#include "DynamicVariable/DynamicConstant.h"
#include "Bindable/Shader.h"
#include "Drawable/Texture.h"

namespace Draw
{
	class MaterialBaseParent
	{
	public:
		template<typename T>
		void SetValue(std::string key1, std::string key2, T value)
		{
			if (bufs.count(key1) == 0) return;
			(*bufs[key1])[key2] = value;
		}

		virtual void Compile() {}
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd) {}
		virtual void Execute(shared_ptr<Graphics::CommandBuffer> cmdbuf_ptr) {}
		void Update(int cur);

		std::map<std::string, shared_ptr<Graphics::Buffer>> buffer_ptrs;
		std::map<std::string, shared_ptr<Dcb::Buffer>> bufs;
		std::shared_ptr<Graphics::DescriptorSetCore> desc_ptr;

	protected:
		void addLayout(std::string key, Dcb::RawLayout&& layout,
			Graphics::LayoutType layoutType, Graphics::DescriptorType descType, Graphics::StageFlag stage);

		void addTexture(Graphics::LayoutType layout_type, Graphics::StageFlag stage, VkImageView textureImageView, VkSampler textureSampler);

		VkPipeline pipeline;
	};
}

#endif // !MATERIAL_BASE_PARENT_H