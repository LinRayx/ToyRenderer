#include "Drawable/OutlineMaterial.h"

namespace Draw
{
	OutlineMaterial::OutlineMaterial(std::shared_ptr<Graphics::Vulkan> vulkan_ptr, std::shared_ptr<Graphics::DescriptorPool> desc_pool_ptr)
		:MaterialBase(vulkan_ptr, desc_pool_ptr)
	{

	}
}