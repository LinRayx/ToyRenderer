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
		};
	public:
		DescriptorSetCore(std::shared_ptr<Vulkan> vulkan_ptr, std::shared_ptr<DescriptorPool> desc_pool_ptr);
		~DescriptorSetCore();
		// void Add(DescriptorType type, StageFlag stage, shared_ptr<Buffer> buffer_ptr);
		void Add(LayoutType layout_type, DescriptorType type, StageFlag stage, shared_ptr<Buffer> buffer_ptr = nullptr);
		void Compile(bool onlyLayout = false);
		bool Update();
	private:
		vector<descInfo> infos;
		vector<VkWriteDescriptorSet> write_sets;
		vector<int> slots_index;

		shared_ptr<DescriptorPool> desc_pool_ptr;
		shared_ptr<Vulkan> vulkan_ptr;
	public:
		// i : swapchain  j : set
		vector<vector<VkDescriptorSet>> descriptorSets;
		shared_ptr<DescriptorSetLayout> desc_layout_ptr;
	};

	class DescriptorSetLayout : public Graphics
	{
		friend class DescriptorSetCore;
		friend class Draw::Drawable;
		friend class CommandBuffer;
	public:
		DescriptorSetLayout(shared_ptr<Vulkan> vulkan_ptr) : vulkan_ptr(vulkan_ptr) {
			layout_bindings.resize(static_cast<size_t>(LayoutType::COUNT));
			descLayouts.resize(static_cast<size_t>(LayoutType::COUNT));
		}
		~DescriptorSetLayout()
		{
			for (auto descriptorSetLayout : descLayouts)
				vkDestroyDescriptorSetLayout( vulkan_ptr->device.device, descriptorSetLayout, vulkan_ptr->device.allocator);
		}
		void add(DescriptorType type, StageFlag stage, vector<VkDescriptorSetLayoutBinding>& bindings);
		void Add(LayoutType layout_type, DescriptorType desc_type, StageFlag stage);
		void Compile();
	private:

		shared_ptr<Vulkan> vulkan_ptr;

		vector<VkDescriptorSetLayout> descLayouts;
		vector<vector<VkDescriptorSetLayoutBinding>> layout_bindings;
	public:
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	};
}

#endif // !DESCRIPTOR_SET_H
