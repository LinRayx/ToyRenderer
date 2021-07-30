#pragma once
#ifndef PIPELINE_LAYOUT_H
#define PIPELINE_LAYOUT_H

#include "Bindable.h"
#include "Pipeline.h"
#include "Vulkan.h"
#include <memory>
#include <vector>
#include "Buffer.h"
using namespace std;

namespace Bind
{
	enum class DESCRIPTOR_TYPE
	{
		UNFIORM,
	};
	class PipelineLayout : public Bindable
	{
	public:
		PipelineLayout(shared_ptr<Graphics::Pipeline> _pipeline_ptr) : pipeline_ptr(_pipeline_ptr) , dstBinding(-1) {}

		void AddLayout(DESCRIPTOR_TYPE type, uint32_t offset, Graphics::Buffer* buffer);

		void Update();

		void Bind(std::shared_ptr<Graphics::Vulkan> _vulkan_ptr) noexcept;
	private:
		uint16_t dstBinding;
		shared_ptr<Graphics::Pipeline> pipeline_ptr;
		vector<VkDescriptorSetLayoutBinding> layout_bindinds;
		vector<VkWriteDescriptorSet> write_sets;
		vector<VkDescriptorBufferInfo> buffer_infos;
		vector<Graphics::Buffer*> buffers;
	};
}

#endif // !UNIFORM_BUFFER_H
