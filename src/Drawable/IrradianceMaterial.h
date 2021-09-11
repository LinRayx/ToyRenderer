#ifndef IRRADIANCE_MATERIAL_H
#define IRRADIANCE_MATERIAL_H

#include "VulkanCore/vulkan_core_headers.h"
#include "DynamicVariable/DynamicConstant.h"
#include "Drawable/MaterialBaseParent.h"
#include "Bindable/VertexBuffer.h"
#include "Bindable/IndexBuffer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Draw
{
	class IrradianceMaterial : public MaterialBaseParent
	{
	public:
		IrradianceMaterial();
		void Execute(shared_ptr<Graphics::CommandBuffer> cmd);
		void BindMeshData(shared_ptr<Bind::VertexBuffer> vBuffer_ptr,
			shared_ptr<Bind::IndexBuffer> iBuffer_ptr);
	protected:
		virtual void initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo);
	private:
		const int32_t dim = 64;
		VkViewport viewport = Graphics::initializers::viewportOffscreen((float)dim, (float)dim, 0.0f, 1.0f);
		VkRect2D scissor = Graphics::initializers::rect2D(dim, dim, 0, 0);
		Dcb::Buffer* pushBlock;
		string irradiance_map_name = "irradiance_map";
	};
}

#endif // !IRRADIANCE_MATERIAL_H
