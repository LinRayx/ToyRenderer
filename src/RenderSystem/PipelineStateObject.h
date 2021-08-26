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
		PipelineStateObject() {
			using namespace Dcb;
			Dcb::VertexBuffer vbuf(
				std::move(
					Dcb::VertexLayout{}
					.Append(VertexLayout::Position3D)
					.Append(VertexLayout::Normal)
					.Append(VertexLayout::Texture2D)
				)
			);
			vBuffer_ptr = make_shared<Bind::VertexBuffer>(vbuf, true);
		}

		virtual void BuildPipeline()
		{}
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd)
		{}
		virtual void Update(int cur)
		{
			for (size_t i = 0; i < models.size(); ++i) {
				models[i]->Update(cur);
			}
		}

		virtual void Add(Draw::Model* model) {
		
		};

		virtual std::vector< Draw::Model* >& GetModels()
		{
			return models;
		}

		virtual void CollectDrawItems()
		{
		}

	protected:
		struct DrawItem
		{
			DrawItem(Draw::Mesh* mesh, Draw::MaterialBase* material)
				: mesh(mesh), material(material) {}
			Draw::Mesh* mesh;
			Draw::MaterialBase* material;
		};

		virtual void buildPipeline() {}
				
		shared_ptr<Bind::VertexShader> vShader_ptr;
		shared_ptr<Bind::PixelShader> pShader_ptr;

		// Layout
		shared_ptr<Graphics::DescriptorSetLayout> desc_layout_ptr;
		shared_ptr<Bind::VertexBuffer> vBuffer_ptr;

		std::vector< Draw::Model* > models;
		std::vector< DrawItem > drawItems;
	};
}

#endif // !PIPELINE_STATE_OBJECT_H
