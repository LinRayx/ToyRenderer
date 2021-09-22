#ifndef BRDF_MATERIAL_H
#define BRDF_MATERIAL_H

#include "Drawable/MaterialBaseParent.h"

namespace Draw
{
	class BrdfMaterial 
	{
	public:
		BrdfMaterial();
		~BrdfMaterial();
		void Compile();
		void Execute(shared_ptr<Graphics::CommandBuffer> cmd);
	private:
		VkPipeline pipeline;
		unique_ptr<Graphics::DescriptorSetCore> desc_ptr;
		const int32_t dim = 512;
	};
}

#endif // !BRDF_MATERIAL_H
