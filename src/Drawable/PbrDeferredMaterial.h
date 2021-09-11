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
		bool SSAO = true;
	protected:
		virtual void initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo);
	};
}

#endif // !PBR_DEFERRED_MATERIAL_H
