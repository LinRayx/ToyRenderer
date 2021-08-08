#ifndef MODEL_H
#define MODEL_H
#include "Vulkan.h"
#include <vector>
#include "DescriptorSet.h"

#include "DynamicConstant.h"
#include "DescriptorPool.h"

namespace Draw
{
	class Model
	{
		friend class Drawable;
	public:
		virtual VkVertexInputBindingDescription getBindingDescription() = 0;
		virtual std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() = 0;
		virtual void Compile() = 0;
		virtual size_t getCount() = 0;
		virtual size_t getSize() = 0;
		virtual void* getData() = 0;
	protected:
		std::shared_ptr<Graphics::Vulkan> vulkan_ptr;
		std::shared_ptr<Graphics::DescriptorSetCore> desc_ptr;
		std::shared_ptr<Graphics::DescriptorPool> desc_pool_ptr;

		Dcb::RawLayout layout;
		shared_ptr<Dcb::Buffer> buf;
	};
}

#endif // !MODEL_H
