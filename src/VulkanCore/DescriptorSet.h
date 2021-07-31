#pragma once
#ifndef DESCRIPTOR_SET_H
#define DESCRIPTOR_SET_H

#include "Buffer.h"
#include <vector>
#include <map>
#include <string>

using namespace std;


namespace Graphics
{
	class DescriptorSet
	{
	public:
		//! Specifies a single descriptor layout and a number of 
		struct descriptor_set_request_t {
			//! The stageFlags member of each entry of bindings is ORed with this value
			//! before using it
			VkShaderStageFlagBits stage_flags;
			//! The minimal number of descriptors per binding. Setting this to one is a
			//! good way to avoid some redundant specifications.
			uint32_t min_descriptor_count;
			//! Number of entries in bindings
			uint32_t binding_count;
			//! A specification of the bindings in the layout. The member binding is
			//! overwritten by the array index before use, stageFlags is ORed with
			//! stage_flags and descriptorCount clamped to a minimum of
			//! min_descriptor_count.
			vector<VkDescriptorSetLayoutBinding> bindings;
		} ;

	public:
		DescriptorSet(string name, uint32_t offset) : dstBinding(-1)
		{
			name_to_dst[name] = ++dstBinding;
			VkDescriptorBufferInfo buffer_info = {};
			buffer_info.offset = 0;
			VkWriteDescriptorSet write_set = {};
			write_set.dstBinding = dstBinding;

			buffer_infos.emplace_back(std::move(buffer_info));
			write_sets.emplace_back(std::move(write_set));
		}
		void Update();
	private:
		uint16_t dstBinding;
		std::vector<VkDescriptorBufferInfo> buffer_infos;
		std::vector<VkWriteDescriptorSet> write_sets;
		
		map<string, uint16_t> name_to_dst;
	};
}

#endif // !DESCRIPTOR_SET_H
