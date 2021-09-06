#ifndef BLUR_MATERIAL_H
#define BLUR_MATERIAL_H

#include "Drawable/MaterialBaseParent.h"

namespace Draw
{
	class BlurMaterial : public MaterialBaseParent
	{
	public:
		BlurMaterial();
		virtual void Compile();
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd);
		void AddBlurMap(string name);
	};
}

#endif // !BLUR_MATERIAL_H
