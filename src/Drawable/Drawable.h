#pragma once
#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <memory>
#include "Bindable_heads.h"
#include "Pipeline.h"
#include "DescriptorSet.h"
#include "RenderPass.h"

using namespace std;

namespace Bind
{
	class Bindable;
	class IndexBuffer;
	class VertexBuffer;
	class PipelineLayout;
	class PixelShader;
	class VertexShader;
}

namespace Draw
{


	enum class GraphicsType
	{
		Vulkan,
		RenderPass,
		Pipeline
	};

	enum class BindType
	{
		IndexBuffer = 0,
		VertexBuffer,
		PixelShader,
		VertexShader,
		PipelineLayout,

	};

	class Drawable
	{

	public:
		Drawable(shared_ptr<Graphics::Vulkan> _vulkan_ptr) : vulkan_ptr(_vulkan_ptr) {
		}

		void Register(GraphicsType type, std::shared_ptr<Graphics::Graphics> elem);
		void Register(BindType type, std::shared_ptr<Bind::Bindable> elem);

		template<GraphicsType type>
		void Register(std::shared_ptr<Graphics::Graphics> elem)
		{
			switch (type)
			{
			case GraphicsType::Vulkan:
				vulkan_ptr = std::dynamic_pointer_cast<Graphics::Vulkan> (elem);
				break;
			case GraphicsType::RenderPass:
				renderpass_ptr = std::dynamic_pointer_cast<Graphics::RenderPass> (elem);
				break;
			case GraphicsType::Pipeline:
				pipeline_ptr = std::dynamic_pointer_cast<Graphics::Pipeline> (elem);
			default:
				break;
			}
		}

		template<BindType type>
		void Register(std::shared_ptr<Bind::Bindable> elem)
		{
			switch (type)
			{
			case Draw::BindType::IndexBuffer:

				break;
			case Draw::BindType::VertexBuffer:
				vertexBuffer_ptr = std::dynamic_pointer_cast<Bind::VertexBuffer>(elem);
				break;
			case Draw::BindType::PixelShader:
				pixelShader_ptr = std::dynamic_pointer_cast<Bind::PixelShader> (elem);
				break;
			case Draw::BindType::VertexShader:
				vertexShader_ptr = std::dynamic_pointer_cast<Bind::VertexShader> (elem);
				break;
			case Draw::BindType::PipelineLayout:
				pipelineLayout_ptr = std::dynamic_pointer_cast<Bind::PipelineLayout> (elem);
				break;
			}
		}
		
		void CompilePipeline();
		// void CompileCommand();
	private:
		shared_ptr<Bind::IndexBuffer> indexBuffer_ptr;
		shared_ptr<Bind::VertexBuffer> vertexBuffer_ptr;
		shared_ptr<Bind::VertexShader> vertexShader_ptr;
		shared_ptr<Bind::PixelShader> pixelShader_ptr;
		shared_ptr<Bind::PipelineLayout> pipelineLayout_ptr;

		shared_ptr<Graphics::Pipeline> pipeline_ptr;
		shared_ptr<Graphics::Vulkan> vulkan_ptr;
		shared_ptr<Graphics::RenderPass> renderpass_ptr;
	private:
		Graphics::Pipeline::pipeline_with_bindings_t pipeline;
		

	};
}

#endif // !DRAWABLE_H


