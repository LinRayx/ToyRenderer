#ifndef PHONE_PSO_H
#define PHONE_PSO_H

#include "RenderSystem/PipelineStateObject.h"


namespace RenderSystem
{
	class PhonePSO : public PipelineStateObject
	{
	public:
		PhonePSO(shared_ptr<Graphics::Vulkan> vulkan_ptr, shared_ptr<Graphics::DescriptorPool> desc_pool_ptr,
			shared_ptr<Graphics::Image> image_ptr);
		void BuildPipeline();
		void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd);
		void Update(shared_ptr<Graphics::CommandBuffer> cmd);
		
		void Add(Bind::VertexBuffer& vbuf, shared_ptr<Graphics::DescriptorSetCore> desc);
	private:

	};
}

#endif // !PHONE_PSO_H
