#ifndef PBR_DEFERRED_MATERIAL_H
#define PBR_DEFERRED_MATERIAL_H

#include "Drawable/MaterialBase.h"

namespace Draw
{
	class PbrDeferredMaterial : public MaterialBaseParent
	{
	public:
		PbrDeferredMaterial();
		virtual void Compile();
		virtual void UpdateSceneData();
		bool SSAO = true;
	};
}

#endif // !PBR_DEFERRED_MATERIAL_H
