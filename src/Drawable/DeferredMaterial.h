#ifndef DEFERRED_MATERIAL_H
#define DEFEREED_MATERIAL_H

#include "Drawable/PBRMaterial.h"
#include <array>

namespace Draw
{
	class DeferredMaterial : public PBRMaterial
	{
	public:
		DeferredMaterial();
		virtual void UpdateSceneData();
	protected:
		virtual void initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo);
	private:
		std::array<VkPipelineColorBlendAttachmentState, 4> blendAttachmentStates = {
			Graphics::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
			Graphics::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
			Graphics::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
			Graphics::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE)
		};
	};
}

#endif // !DEFERRED_MATERIAL_H
