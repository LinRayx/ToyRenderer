#ifndef PIPELINE_STATE_OBJECT_H
#define PIPELINE_STATE_OBJECT_H

#include "Bindable/VertexShader.h"
#include "Bindable/PixelShader.h"
#include "Bindable/VertexBuffer.h"
#include "Bindable/IndexBuffer.h"

#include "VulkanCore/vulkan_core_headers.h"


#include "DynamicVariable/DynamicVertex.h"

#include "Drawable/Model.h"

#include <memory>
#include <vector>

using namespace std;

namespace RenderSystem
{
	class PipelineStateObject
	{
	public:
		PipelineStateObject() {}

		virtual void BuildPipeline()
		{}
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd)
		{}
		virtual void Update(int cur)
		{}
		virtual void Add(Bind::VertexBuffer& vbuf, Graphics::DescriptorSetCore& desc)
		{}

		virtual void Add(Draw::Model* model) {};

		virtual std::vector< Draw::Model* >& GetModels()
		{
			std::vector < Draw::Model*> v;
			return v;
		}

	protected:

		virtual void buildPipeline(Draw::DrawItem* item) {}
				
		shared_ptr<Bind::VertexShader> vShader_ptr;
		shared_ptr<Bind::PixelShader> pShader_ptr;

		// Layout
		shared_ptr<Graphics::DescriptorSetLayout> desc_layout_ptr;
		shared_ptr<Bind::VertexBuffer> vBuffer_ptr;

		std::vector< Draw::Model* > models;
		std::vector< Draw::DrawItem* > drawItems;
	};
}

#endif // !PIPELINE_STATE_OBJECT_H
