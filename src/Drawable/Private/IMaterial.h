#ifndef I_MATERIAL_H
#define I_MATERIAL_H
#include "VulkanCore/vulkan_core_headers.h"

namespace Draw
{
	class IMaterial
	{
	public:
		virtual void UpdateSceneData() = 0;
		virtual void Compile() = 0;
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd) = 0;
		virtual void Execute(shared_ptr<Graphics::CommandBuffer> cmdbuf_ptr) = 0;
		virtual bool SetUI() = 0;
	};
}

#endif // !I_MATERIAL_H
