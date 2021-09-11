#ifndef BLUR_MATERIAL_H
#define BLUR_MATERIAL_H

#include "Drawable/MaterialBaseParent.h"

namespace Draw
{
	class BlurMaterial : public MaterialBaseParent
	{
	public:
		BlurMaterial();
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd);
		void AddBlurMap(string name);
	protected:
		virtual void initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo) override;
	};
}

#endif // !BLUR_MATERIAL_H
