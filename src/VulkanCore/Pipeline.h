#pragma once
#ifndef PIPELINE_H
#define PIPELINE_H

#include "Vulkan.h"
#include "DescriptorSet.h"

namespace Graphics
{
	class Pipeline
	{
		public:
		/*! This structure combines a pipeline state object with everything that is
			needed to construct descriptor sets for it and the descriptor sets
			themselves.*/
		 struct pipeline_with_bindings_t {
			//! Descriptor layout used by pipeline_layout
			VkDescriptorSetLayout descriptor_set_layout;
			//! Pipeline layout used by pipeline
			VkPipelineLayout pipeline_layout;
			//! Descriptor pool used to allocate descriptor_sets. It matches
			//! descriptor_set_layout.
			VkDescriptorPool descriptor_pool;
			//! An array with one descriptor set per swapchain image
			VkDescriptorSet* descriptor_sets;
			//! The pipeline state
			VkPipeline pipeline;
		} ;
	public:
		/*! Creates a single descriptor set layout with a pipeline layout using only
		that layout and allocates the requested number of descriptor sets by means
		of a newly created descriptor pool. The pipeline is not created and assumed
		to be not created yet. The layout is defined by the given request.
		\return 0 if all objects were created / allocated successfully.*/
		int create_descriptor_sets(Pipeline::pipeline_with_bindings_t* pipeline, const DescriptorSet::descriptor_set_request_t* request, uint32_t descriptor_set_count);

		/*! This little utility helps with writing to descriptor sets. For each entry
			of the given array, it:
			- sets sType to VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			- sets descriptorType to the corresponding value in request (based on
			  dstBinding, if any),
			- sets descriptorCount to the corresponding value in request or to
			  request->min_descriptor_count if that is bigger.*/
		void complete_descriptor_set_write(uint32_t write_count, vector<VkWriteDescriptorSet>& writes, const DescriptorSet::descriptor_set_request_t* request);
		//! Frees objects and zeros
		void destroy_pipeline_with_bindings(pipeline_with_bindings_t* pipeline);
	private:
		/*! Creates one or more buffers according to the given specifications, performs
			a single memory allocation for all of them and binds it.
			\param buffers The output object. Use destroy_buffers() to free it.
			\param device The used device.
			\param buffer_infos A specification of each buffer that is to be created
				(buffer_count in total).
			\param buffer_count The number of buffers to create.
			\param memory_properties The memory flags that you want to enforce for the
				memory allocation. Combination of VkMemoryHeapFlagBits.
			\return 0 on success.*/
	private:
		shared_ptr<Vulkan> vulkan_ptr;
	public:
		pipeline_with_bindings_t pipeline;
	};
}

#endif // !PIPELINE_H
