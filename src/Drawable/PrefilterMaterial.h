#ifndef PRE_FILTER_MATERIAL_H
#define PRE_FILTER_MATERIAL_H

#include "VulkanCore/vulkan_core_headers.h"
#include "DynamicVariable/DynamicConstant.h"
#include "Drawable/MaterialBaseParent.h"
#include "Bindable/VertexBuffer.h"
#include "Bindable/IndexBuffer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Draw
{
	class PrefilterMaterial : public MaterialBaseParent
	{
	public:
		PrefilterMaterial();
		virtual void Execute(shared_ptr<Graphics::CommandBuffer> cmdBuf);
	protected:
		virtual void initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo);
	private:
		const int32_t dim = 512;
		const int32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;
		Dcb::Buffer* pushBlock;
		string framebuffer_attachment = "prefilter_attachment";
		string resources = "prefilter_map";
	};
}

#endif