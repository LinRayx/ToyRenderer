#pragma once
#ifndef PIPELINE_H
#define PIPELINE_H

#include "Vulkan.h"
#include "DescriptorSet.h"

namespace Graphics
{
	class Pipeline : public Graphics
	{
	public:

		Pipeline(shared_ptr<Vulkan> _vulkan_ptr) : vulkan_ptr(_vulkan_ptr) {}
		~Pipeline();

	private:
		shared_ptr<Vulkan> vulkan_ptr;

	public:
		
		VkPipeline pipeline;
	};
}

#endif // !PIPELINE_H
