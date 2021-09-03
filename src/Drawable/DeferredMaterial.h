#ifndef DEFERRED_MATERIAL_H
#define DEFEREED_MATERIAL_H

#include "Drawable/MaterialBase.h"
#include <array>

namespace Draw
{
	class DeferredMaterial : public MaterialBase
	{
	public:
		DeferredMaterial();
		virtual void Compile();
		virtual void UpdateSceneData();
	private:

	};
}

#endif // !DEFERRED_MATERIAL_H
