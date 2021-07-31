#include "Pipeline.h"

namespace Graphics
{
	int Pipeline::create_descriptor_sets(pipeline_with_bindings_t* pipeline, const DescriptorSet::descriptor_set_request_t* request, uint32_t descriptor_set_count) {
		auto& device = vulkan_ptr->device;
		memset(pipeline, 0, sizeof(*pipeline));
		// Copy and complete the bindings
		VkDescriptorSetLayoutBinding* bindings = (VkDescriptorSetLayoutBinding*)malloc(sizeof(VkDescriptorSetLayoutBinding) * request->binding_count);
		for (uint32_t i = 0; i != request->binding_count; ++i) {
			bindings[i] = request->bindings[i];
			bindings[i].binding = i;
			bindings[i].stageFlags |= request->stage_flags;
			bindings[i].descriptorCount = (bindings[i].descriptorCount < request->min_descriptor_count)
				? request->min_descriptor_count
				: bindings[i].descriptorCount;
		}
		// Create the descriptor set layout
		VkDescriptorSetLayoutCreateInfo descriptor_layout_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = request->binding_count,
			.pBindings = bindings
		};
		if (vkCreateDescriptorSetLayout(device.device, &descriptor_layout_info, device.allocator, &pipeline->descriptor_set_layout)) {
			printf("Failed to create a descriptor set layout.\n");
			free(bindings);
			destroy_pipeline_with_bindings(pipeline);
			return 1;
		}

		// Create the pipeline layout using only this descriptor layout
		VkPipelineLayoutCreateInfo pipeline_layout_info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &pipeline->descriptor_set_layout
		};
		if (vkCreatePipelineLayout(device.device, &pipeline_layout_info, device.allocator, &pipeline->pipeline_layout)) {
			printf("Failed to create a pipeline layout from a single descriptor set layout.\n");
			free(bindings);
			destroy_pipeline_with_bindings(pipeline);
			return 1;
		}

		// Create a descriptor pool with the requested number of descriptor sets
		VkDescriptorPoolSize* descriptor_pool_sizes = (VkDescriptorPoolSize*)malloc(request->binding_count * sizeof(VkDescriptorPoolSize));
		for (uint32_t i = 0; i != request->binding_count; ++i) {
			descriptor_pool_sizes[i].type = bindings[i].descriptorType;
			descriptor_pool_sizes[i].descriptorCount = bindings[i].descriptorCount * descriptor_set_count;
		}
		free(bindings);
		VkDescriptorPoolCreateInfo descriptor_pool_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = descriptor_set_count,
			.poolSizeCount = request->binding_count,
			.pPoolSizes = descriptor_pool_sizes
		};
		if (vkCreateDescriptorPool(device.device, &descriptor_pool_info, device.allocator, &pipeline->descriptor_pool)) {
			printf("Failed to create a descriptor pool to allocate %u descriptor sets.\n", descriptor_set_count);
			free(descriptor_pool_sizes);
			destroy_pipeline_with_bindings(pipeline);
			return 1;
		}
		free(descriptor_pool_sizes);
		VkDescriptorSetLayout* layouts = (VkDescriptorSetLayout*)malloc(sizeof(VkDescriptorSetLayout) * descriptor_set_count);
		for (uint32_t i = 0; i != descriptor_set_count; ++i)
			layouts[i] = pipeline->descriptor_set_layout;
		VkDescriptorSetAllocateInfo descriptor_alloc_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = pipeline->descriptor_pool,
			.descriptorSetCount = descriptor_set_count,
			.pSetLayouts = layouts
		};
		pipeline->descriptor_sets = (VkDescriptorSet*)malloc(sizeof(VkDescriptorSet) * descriptor_set_count);
		if (vkAllocateDescriptorSets(device.device, &descriptor_alloc_info, pipeline->descriptor_sets)) {
			printf("Failed to create %u descriptor sets.\n", descriptor_alloc_info.descriptorSetCount);
			free(layouts);
			destroy_pipeline_with_bindings(pipeline);
			return 1;
		}
		free(layouts);
		return 0;
	}


	void Pipeline::complete_descriptor_set_write(uint32_t write_count, vector<VkWriteDescriptorSet>& writes, const DescriptorSet::descriptor_set_request_t* request) {
		for (uint32_t i = 0; i != write_count; ++i) {
			writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			if (writes[i].dstBinding < request->binding_count) {
				writes[i].descriptorType = request->bindings[writes[i].dstBinding].descriptorType;
				writes[i].descriptorCount = request->bindings[writes[i].dstBinding].descriptorCount;
			}
			writes[i].descriptorCount = (writes[i].descriptorCount < request->min_descriptor_count)
				? request->min_descriptor_count
				: writes[i].descriptorCount;
		}
	}


	void Pipeline::destroy_pipeline_with_bindings(pipeline_with_bindings_t* pipeline) {
		auto& device = vulkan_ptr->device;
		if (pipeline->pipeline)
			vkDestroyPipeline(device.device, pipeline->pipeline, device.allocator);
		if (pipeline->descriptor_pool)
			vkDestroyDescriptorPool(device.device, pipeline->descriptor_pool, device.allocator);
		free(pipeline->descriptor_sets);
		if (pipeline->pipeline_layout)
			vkDestroyPipelineLayout(device.device, pipeline->pipeline_layout, device.allocator);
		if (pipeline->descriptor_set_layout)
			vkDestroyDescriptorSetLayout(device.device, pipeline->descriptor_set_layout, device.allocator);
		memset(pipeline, 0, sizeof(*pipeline));
	}
}