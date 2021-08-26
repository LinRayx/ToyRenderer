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
		static DescriptorPool* getInstance() {
			if (instance == NULL) {
				instance = new DescriptorPool();
			}
			return instance;
		}

		VkDescriptorPool GetPool()
		{
			return descriptorPool;
		}
	private:
		DescriptorPool();
		~DescriptorPool();
		class Deletor {
		public:
			~Deletor() {
				if (DescriptorPool::instance != NULL)
					delete DescriptorPool::instance;
			}
		};
		static Deletor deletor;

		static DescriptorPool* instance;
		VkDescriptorPool descriptorPool;
	};
}

#endif // !DESCRIPTOR_POOL_H