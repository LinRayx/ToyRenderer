#ifndef COMPOSITION_MATERIAL_H
#define COMPOSITION_MATERIAL_H

#include "Drawable/MaterialBase.h"

namespace Draw
{
	class CompositionMaterial : public MaterialBase
	{
	public:
		CompositionMaterial();
		void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd);
	protected:
		virtual void initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo);
	};
}

#endif // !COMPOSITION_MATERIAL_H
