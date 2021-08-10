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
		slots_index.resize(static_cast<size_t>(LayoutType::COUNT), 0);
	}

	DescriptorSetCore::~DescriptorSetCore()
	{

	}

	void DescriptorSetCore::Add(LayoutType layout_type, DescriptorType type, StageFlag stage, shared_ptr<Buffer> buffer_ptr)
	{
		desc_layout_ptr->Add(layout_type, type, stage);

		if (buffer_ptr == nullptr) return;

		uint16_t index = static_cast<uint16_t>(layout_type);
		descInfo info{ index, type, buffer_ptr, slots_index[index]++ };
		infos.emplace_back(std::move(info));
	}

	void DescriptorSetCore::Compile(bool onlyLayout)
	{
		desc_layout_ptr->Compile();

		if (onlyLayout) return;

		descriptorSets.resize(vulkan_ptr->swapchain.image_count);
		for (size_t i = 0; i < descriptorSets.size(); ++i) {
			descriptorSets[i].resize(static_cast<size_t>(LayoutType::COUNT));
			for (size_t j = 0; j < descriptorSets[i].size(); ++j) {

				std::vector<VkDescriptorSetLayout> layouts(1, desc_layout_ptr->descLayouts[j]);
				VkDescriptorSetAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = desc_pool_ptr->descriptorPool;
				allocInfo.descriptorSetCount = 1;
				allocInfo.pSetLayouts = layouts.data();

				if (vkAllocateDescriptorSets(vulkan_ptr->device.device, &allocInfo, &descriptorSets[i][j]) != VK_SUCCESS) {
					throw std::runtime_error("failed to allocate descriptor sets!");
				}
			}
		}

		for (size_t i = 0; i < vulkan_ptr->swapchain.image_count; ++i) {
			write_sets.clear();
			vector<VkDescriptorBufferInfo> bufferInfos(infos.size());
			for (size_t j = 0; j < infos.size(); ++j) {
				//if (infos[j].buffer_ptr->update[i] == false) continue;
				//infos[j].buffer_ptr->update[i] = true;
				bufferInfos[j].buffer = infos[j].buffer_ptr->buffers[i];
				bufferInfos[j].offset = 0;
				bufferInfos[j].range = infos[j].buffer_ptr->size;

				VkWriteDescriptorSet descriptorWrite = {};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = descriptorSets[i][infos[j].slot];
				descriptorWrite.dstBinding = infos[j].binding;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = &bufferInfos[j];

				write_sets.emplace_back(std::move(descriptorWrite));
			}	
			vkUpdateDescriptorSets(vulkan_ptr->device.device, static_cast<uint32_t>(write_sets.size()), write_sets.data(), 0, nullptr);
		}

	}

	bool DescriptorSetCore::Update()
	{
		for (size_t i = 0; i < infos.size(); ++i) {
			if (infos[i].buffer_ptr->update == true) return true;
		}
		return false;
	}

	void DescriptorSetLayout::add(DescriptorType type, StageFlag stage, vector<VkDescriptorSetLayoutBinding>& bindings)
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

	void DescriptorSetLayout::Add(LayoutType layout_type, DescriptorType desc_type, StageFlag stage)
	{
		uint32_t index = static_cast<uint32_t>(layout_type);
		add(desc_type, stage, layout_bindings[index]);
	}

	void DescriptorSetLayout::Compile()
	{
		for (size_t i = 0; i < descLayouts.size(); ++i) {
			VkDescriptorSetLayoutCreateInfo layoutInfo = {};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<uint32_t>(layout_bindings[i].size());
			layoutInfo.pBindings = layout_bindings[i].data();

			if (vkCreateDescriptorSetLayout(vulkan_ptr->device.device, &layoutInfo, nullptr, &descLayouts[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor set layout!");
			}
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descLayouts.data();

		if (vkCreatePipelineLayout(vulkan_ptr->device.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

}