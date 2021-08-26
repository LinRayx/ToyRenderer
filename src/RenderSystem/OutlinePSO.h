#ifndef OUTLINE_PSO_H
#define OUTLINE_PSO_H

#include "RenderSystem/PipelineStateObject.h"
#include "VulkanCore/VulkanInitalizers.hpp"

namespace RenderSystem
{
	// 1. ��Ⱦ���壬ͬʱд��ģ�建�壬stencil pass ͨ����д�룬����������Ȳ���
	// 2. ��ֹд��ģ�建�壬������Ȳ��ԣ��Ŵ�һ�����壬��Ⱦ�߿�
	// 3. ��������ģ�����Ȼ���
	class OutlinePSO : public PipelineStateObject
	{
	public:
		OutlinePSO();
		~OutlinePSO();

		virtual void BuildPipeline() override;
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd) override;

		virtual void Add(Draw::Model* model) override;
		void CollectDrawItems();
	private:
		Draw::MaterialType matType = Draw::MaterialType::Outline;

		VkPipeline pipelineOutline;
	};
}

#endif