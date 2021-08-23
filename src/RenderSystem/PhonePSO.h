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
		~PhonePSO();
		virtual void BuildPipeline() override;
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd) override;
		
		void Update(int cur);
		void Add(Draw::Model* model);
		std::vector< Draw::Model* >& GetModels();
	private:

	};
}

#endif // !PHONE_PSO_H
