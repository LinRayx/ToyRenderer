#pragma once
#ifndef PIPELINE_LAYOUT_H
#define PIPELINE_LAYOUT_H

#include "Bindable.h"
#include "Pipeline.h"
#include "Vulkan.h"
#include <memory>
#include <vector>
#include "DescriptorSet.h"

using namespace std;

namespace Draw {
	class Drawable;
}

namespace Bind
{
	enum class DESCRIPTOR_TYPE
	{
		UNFIORM,
	};
	class PipelineLayout : public Bindable
	{
		
	public:
		friend class Draw::Drawable;
		PipelineLayout() :  dstBinding(-1) {}

		void AddLayout(DESCRIPTOR_TYPE type, uint32_t offset);

	private:
		uint16_t dstBinding;
		vector<VkDescriptorSetLayoutBinding> layout_bindinds;
	};
}

#endif // !UNIFORM_BUFFER_H
