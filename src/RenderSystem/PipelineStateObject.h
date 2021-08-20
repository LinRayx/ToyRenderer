#ifndef PIPELINE_STATE_OBJECT_H
#define PIPELINE_STATE_OBJECT_H

#include "Bindable/VertexShader.h"
#include "Bindable/PixelShader.h"
#include "Bindable/VertexBuffer.h"

#include "VulkanCore/RenderPass.h"
#include "VulkanCore/DescriptorSet.h"
#include "VulkanCore/Pipeline.h"
#include "VulkanCore/CommandBuffer.h"

#include "DynamicVariable/DynamicVertex.h"

#include "Drawable/Material.h"
#include "Drawable/Model.h"

#include <memory>
#include <vector>

using namespace std;

namespace RenderSystem
{
	class PipelineStateObject
	{
	public:
		PipelineStateObject(shared_ptr<Graphics::Vulkan> vulkan_ptr) : vulkan_ptr(vulkan_ptr) {}

		virtual void BuildPipeline()
		{}
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd)
		{}
		virtual void Update(int cur)
		{}
		virtual void Add(Bind::VertexBuffer& vbuf, Graphics::DescriptorSetCore& desc)
		{}

	protected:
		shared_ptr<Graphics::Vulkan> vulkan_ptr;
		
		shared_ptr<Graphics::RenderPass> renderpass_ptr;
		shared_ptr<Graphics::Pipeline> pipeline_ptr;
		

		shared_ptr<Bind::VertexShader> vShader_ptr;
		shared_ptr<Bind::PixelShader> pShader_ptr;

		// Layout
		shared_ptr<Graphics::DescriptorSetLayout> desc_layout_ptr;
		shared_ptr<Bind::VertexBuffer> vBuffer_ptr;

		std::vector< Draw::Model* > models;
	};
}

#endif // !PIPELINE_STATE_OBJECT_H
