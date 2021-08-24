#ifndef OUTLINE_MATERIAL
#define OUTLINE_MATERIAL

#include "Drawable/MaterialBase.h"

namespace Draw
{
	class OutlineMaterial : public MaterialBase
	{
	public:
		OutlineMaterial(std::shared_ptr<Graphics::Vulkan> vulkan_ptr, std::shared_ptr<Graphics::DescriptorPool> desc_pool_ptr);
	};
}

#endif // !OUTLINE_MATERIAL
