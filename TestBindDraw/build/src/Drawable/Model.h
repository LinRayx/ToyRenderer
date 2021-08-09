#ifndef MODEL_H
#define MODEL_H
#include "Vulkan.h"
#include <vector>
#include "DescriptorSet.h"
#include "DynamicConstant.h"
#include "DescriptorPool.h"
#include <glm/glm.hpp>
#include "Camera.h"
#include <map>
#include <string>

namespace Draw
{
	class Model
	{
		friend class Drawable;
	public:
		virtual VkVertexInputBindingDescription getBindingDescription() = 0;
		virtual std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() = 0;
		virtual void Compile() = 0;
		virtual void Update() = 0;
		virtual size_t getCount() = 0;
		virtual size_t getSize() = 0;
		virtual void* getData() = 0;
	protected:
		struct Transform_S {
			glm::vec3 translate;
			glm::vec3 rotate;
			glm::vec3 scale;
		} transform_s;

		glm::mat4 getModelMatrix();

		std::shared_ptr<Graphics::Vulkan> vulkan_ptr;
		std::shared_ptr<Graphics::DescriptorSetCore> desc_ptr;
		std::shared_ptr<Graphics::DescriptorPool> desc_pool_ptr;
		std::shared_ptr<Control::Camera> camera_ptr;

		std::map<std::string, shared_ptr<Graphics::Buffer>> buffer_ptrs;
		std::map<std::string, shared_ptr<Dcb::Buffer>> bufs;
	};
}

#endif // !MODEL_H
