#pragma once

#include "DescriptorSet.h"
#include <stdexcept>

namespace Graphics
{
	DescriptorSetCore::DescriptorSetCore(std::shared_ptr<Vulkan> vulkan_ptr, std::shared_ptr<DescriptorPool> desc_pool_ptr)
		: vulkan_ptr(vulkan_ptr), desc_pool_ptr(desc_pool_ptr)
	{
		desc_layout_ptr = make_shared<DescriptorSetLayout>(vulkan_ptr);
		write_sets.clear();
		descriptorSets.clear();
	}

	DescriptorSetCore::~DescriptorSetCore()
	{

	}

	void DescriptorSetCore::Add(DescriptorType type, StageFlag stage, shared_ptr<Buffer> buffer_ptr)
	{
		desc_layout_ptr->Add(type, stage);
		descInfo info{ type, buffer_ptr };
		infos.emplace_back(std::move(info));
	}

	void DescriptorSetCore::Compile()
	{
		desc_layout_ptr->Compile();

		std::vector<VkDescriptorSetLayout> layouts(vulkan_ptr->swapchain.image_count, desc_layout_ptr->descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = desc_pool_ptr->descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(vulkan_ptr->swapchain.image_count);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(vulkan_ptr->swapchain.image_count);
		if (vkAllocateDescriptorSets(vulkan_ptr->device.device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < vulkan_ptr->swapchain.image_count; ++i) {
			write_sets.clear();
			for (size_t j = 0; j < infos.size(); ++j) {
				//if (infos[j].buffer_ptr->update[i] == false) continue;
				//infos[j].buffer_ptr->update[i] = true;
				VkDescriptorBufferInfo bufferInfo = {};

				bufferInfo.buffer = infos[j].buffer_ptr->buffers[0];
				bufferInfo.offset = 0;
				bufferInfo.range = infos[j].buffer_ptr->size;

				VkWriteDescriptorSet descriptorWrite = {};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = descriptorSets[i];
				descriptorWrite.dstBinding = static_cast<uint32_t>(j);
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = &bufferInfo;

				write_sets.emplace_back(std::move(descriptorWrite));
				
			}	
			vkUpdateDescriptorSets(vulkan_ptr->device.device, static_cast<uint32_t>(write_sets.size()), write_sets.data(), 0, nullptr);
		}

	}

	void DescriptorSetLayout::Add(DescriptorType type, StageFlag stage)
	{
		switch (type)
		{
		case DescriptorType::UNIFORM:
			VkDescriptorSetLayoutBinding uboLayoutBinding{};
			uboLayoutBinding.binding = static_cast<uint32_t>(bindings.size());
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.descriptorType = Graphics::GetDescriptorType(type);
			uboLayoutBinding.pImmutableSamplers = nullptr;
			uboLayoutBinding.stageFlags = Graphics::GetStageFlag(stage);
			bindings.emplace_back(uboLayoutBinding);
			break;
		}

	}

	void DescriptorSetLayout::Compile()
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(vulkan_ptr->device.device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

		if (vkCreatePipelineLayout(vulkan_ptr->device.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

}