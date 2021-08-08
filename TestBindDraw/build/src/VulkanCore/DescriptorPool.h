#ifndef DESCRIPTOR_POOL_H
#define DESCRIPTOR_POOL_H

#include "Vulkan.h"
#include <memory>

namespace Graphics
{
	class DescriptorPool : public Graphics
	{
		friend class DescriptorSetCore;
	public:
		DescriptorPool(std::shared_ptr<Vulkan> vulkan_ptr);
		~DescriptorPool();
	private:
		VkDescriptorPool descriptorPool;
		std::shared_ptr<Vulkan> vulkan_ptr;
	};
}

#endif // !DESCRIPTOR_POOL_H