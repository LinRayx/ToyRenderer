#ifndef PBR_DEFERRED_MATERIAL_H
#define PBR_DEFERRED_MATERIAL_H

#include "Drawable/MaterialBase.h"

namespace Draw
{
	class PbrDeferredMaterial : public MaterialBaseParent
	{
	public:
		PbrDeferredMaterial();
		virtual void UpdateSceneData();
		void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd);
		bool SSAO = true;
	protected:
		virtual void initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo);
	private:
		std::array<VkPipelineColorBlendAttachmentState, 2> blendAttachmentStates = {
			Graphics::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
			Graphics::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		};
	};
}

#endif // !PBR_DEFERRED_MATERIAL_H
