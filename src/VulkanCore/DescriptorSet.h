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
			DescriptorType type;
			shared_ptr<Buffer> buffer_ptr;
		};
	public:
		DescriptorSetCore(std::shared_ptr<Vulkan> vulkan_ptr, std::shared_ptr<DescriptorPool> desc_pool_ptr);
		~DescriptorSetCore();
		void Add(DescriptorType type, StageFlag stage, shared_ptr<Buffer> buffer_ptr);
		void Compile();

	private:
		
		vector<descInfo> infos;

		std::vector<VkWriteDescriptorSet> write_sets;
		std::vector<VkDescriptorSet> descriptorSets;

		std::shared_ptr<DescriptorPool> desc_pool_ptr;
		std::shared_ptr<Vulkan> vulkan_ptr;
		std::shared_ptr<DescriptorSetLayout> desc_layout_ptr;
	};

	class DescriptorSetLayout : public Graphics
	{
		friend class DescriptorSetCore;
		friend class Draw::Drawable;
		friend class CommandBuffer;
	public:
		DescriptorSetLayout(shared_ptr<Vulkan> vulkan_ptr) : vulkan_ptr(vulkan_ptr) {}
		~DescriptorSetLayout()
		{
			vkDestroyDescriptorSetLayout( vulkan_ptr->device.device, descriptorSetLayout, vulkan_ptr->device.allocator);
		}
		void Add(DescriptorType type, StageFlag stage);
		void Compile();
	private:
		vector<VkDescriptorSetLayoutBinding> bindings;
		VkDescriptorSetLayout descriptorSetLayout;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		shared_ptr<Vulkan> vulkan_ptr;
	};
}

#endif // !DESCRIPTOR_SET_H
