#ifndef PHONE_PSO_H
#define PHONE_PSO_H

#include "RenderSystem/PipelineStateObject.h"


namespace RenderSystem
{
	class PhonePSO : public PipelineStateObject
	{
	public:
		PhonePSO();
		~PhonePSO();
		virtual void BuildPipeline() override;
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd) override;
		
		virtual void Update(int cur) override;
		virtual void Add(Draw::Model* model) override;
		virtual void CollectDrawItems() override;
		std::vector< Draw::Model* >& GetModels();
	protected:
		virtual void buildPipeline() override;
	private:
		Draw::MaterialType matType = Draw::MaterialType::Phone;
		VkPipeline pipeline;
	};
}

#endif // !PHONE_PSO_H
