#pragma once
#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <memory>
#include "Bindable_heads.h"
#include "Pipeline.h"
#include "DescriptorSet.h"
#include "RenderPass.h"
#include "CommandBuffer.h"
#include "CommandQueue.h"
#include "Model.h"

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
		Pipeline,
		CommandBuffer,
		CommandQueue,
		DescriptorSet,
	};

	enum class BindType
	{
		IndexBuffer = 0,
		VertexBuffer,
		PixelShader,
		VertexShader,
		

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
			case GraphicsType::CommandBuffer:
				cmdBuf_ptr = std::dynamic_pointer_cast<Graphics::CommandBuffer> (elem);
				break;
			case GraphicsType::CommandQueue:
				cmdQueue_ptr = std::dynamic_pointer_cast<Graphics::CommandQueue> (elem);
				break;
			case GraphicsType::DescriptorSet:

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
			}
		}

		void Register(shared_ptr<Model> model_ptr)
		{
			this->model_ptr = model_ptr;
		}

		void CompilePipeline();
		// void CompileCommand();
		void BuildCommandBuffer()
		{
			cmdBuf_ptr->BuildCommandBuffer(renderpass_ptr, pipeline_ptr, vertexBuffer_ptr->buffer_ptr, model_ptr->desc_ptr);
		}

		void Submit()
		{
			cmdQueue_ptr->SetCommandBuffer(cmdBuf_ptr);
			cmdQueue_ptr->Submit();
		}
	private:
		shared_ptr<Bind::IndexBuffer> indexBuffer_ptr;
		shared_ptr<Bind::VertexBuffer> vertexBuffer_ptr;
		shared_ptr<Bind::VertexShader> vertexShader_ptr;
		shared_ptr<Bind::PixelShader> pixelShader_ptr;

		shared_ptr<Graphics::Pipeline> pipeline_ptr;
		shared_ptr<Graphics::Vulkan> vulkan_ptr;
		shared_ptr<Graphics::RenderPass> renderpass_ptr;
		shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr;
		shared_ptr<Graphics::CommandQueue> cmdQueue_ptr;

		shared_ptr<Model> model_ptr;
	private:
		// Graphics::Pipeline::pipeline_with_bindings_t pipeline;
	};
}

#endif // !DRAWABLE_H


