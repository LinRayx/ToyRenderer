#ifndef BRDF_MATERIAL_H
#define BRDF_MATERIAL_H

#include "Drawable/MaterialBaseParent.h"

namespace Draw
{
	class BrdfMaterial 
	{
	public:
		BrdfMaterial();
		void Compile();
		void BuildCmd(shared_ptr<Graphics::CommandBuffer> cmd);
	private:
		VkPipeline pipeline;
		shared_ptr<Graphics::DescriptorSetLayout> desc_layout_ptr;
		const int32_t dim = 512;
	};
}

#endif // !BRDF_MATERIAL_H
