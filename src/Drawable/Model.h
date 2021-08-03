#ifndef MODEL_H
#define MODEL_H
#include "Vulkan.h"
#include <vector>

namespace Draw
{
	class Model
	{
	public:
		virtual VkVertexInputBindingDescription getBindingDescription() = 0;
		virtual std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() = 0;
		virtual size_t getCount() = 0;
		virtual size_t getSize() = 0;
		virtual void* getData() = 0;
	};
}

#endif // !MODEL_H
