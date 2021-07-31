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
		void Register(GraphicsType type, std::shared_ptr<Graphics::Graphics> elem);
		void Register(BindType type, std::shared_ptr<Bind::Bindable> elem);
		void CompilePipeline();
		void CompileCommand();
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


