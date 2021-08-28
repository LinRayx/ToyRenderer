// DescriptorSetLayout 和 DescriptorSet 需要分开
// PSO中单独创建DescriptorSetLayout
// Material 中拥有各自的DescriptorSet，然后绑定到PSO中
// 在Build PSO的时候，同时Build每个Material的DescriptorSet

#pragma once
#ifndef DESCRIPTOR_SET_H
#define DESCRIPTOR_SET_H

#include "Buffer.h"
#include "DescriptorPool.h"
#include "Pipeline.h"
#include <vector>
#include <map>
#include <string>
#include <memory>

using namespace std;

namespace Draw
{
	class Drawable;
}

namespace Graphics
{
	class DescriptorSetLayout;

	class DescriptorSetCore : public Graphics
	{
		friend class Draw::Drawable;
		friend class CommandBuffer;

		struct descInfo
		{
			uint16_t slot;
			DescriptorType type;
			shared_ptr<Buffer> buffer_ptr;
			uint16_t binding;

			VkImageView textureImageView;
			VkSampler textureSampler;
		};
	public:
		DescriptorSetCore();
		~DescriptorSetCore();

		void Add(LayoutType layout_type, DescriptorType type, StageFlag stage, shared_ptr<Buffer> buffer_ptr);
		void Add(LayoutType layout_type, DescriptorType type, StageFlag stage, VkImageView textureImageView, VkSampler textureSampler);
		void Add(LayoutType layout_type, DescriptorType type, StageFlag stage);
		void Compile(shared_ptr<DescriptorSetLayout> desc_layout_ptr);
		bool Update();
	private:
		vector<descInfo> infos;
		vector<VkWriteDescriptorSet> write_sets;
		vector<uint16_t> slots_index;

	public:
		// i : swapchain  j : set
		vector<vector<VkDescriptorSet>> descriptorSets;
		
	};

	class DescriptorSetLayout : public Graphics
	{
		friend class DescriptorSetCore;
		friend class Draw::Drawable;
		friend class CommandBuffer;
	public:
		DescriptorSetLayout()  {
			layout_bindings.resize(static_cast<size_t>(LayoutType::COUNT));
			descLayouts.resize(static_cast<size_t>(LayoutType::COUNT));
		}
		~DescriptorSetLayout()
		{
			for (auto descriptorSetLayout : descLayouts)
				vkDestroyDescriptorSetLayout( Vulkan::getInstance()->device.device, descriptorSetLayout, Vulkan::getInstance()->device.allocator);
		}
		void add(DescriptorType type, StageFlag stage, vector<VkDescriptorSetLayoutBinding>& bindings);
		void Add(LayoutType layout_type, DescriptorType desc_type, StageFlag stage);
		void Compile();
	private:
		vector<VkDescriptorSetLayout> descLayouts;
		vector<vector<VkDescriptorSetLayoutBinding>> layout_bindings;
	public:
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	};
}

#endif // !DESCRIPTOR_SET_H
