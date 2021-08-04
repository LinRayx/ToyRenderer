#pragma once
#include "Drawable.h"

namespace Draw
{
	void Drawable::Register(GraphicsType type, std::shared_ptr<Graphics::Graphics> elem)
	{
		switch (type)
		{
		case Draw::GraphicsType::Vulkan:
			vulkan_ptr = std::dynamic_pointer_cast<Graphics::Vulkan> (elem);
			break;
		case Draw::GraphicsType::RenderPass:
			renderpass_ptr = std::dynamic_pointer_cast<Graphics::RenderPass> (elem);
			break;
		default:
			break;
		}
	}


	void Drawable::Register(BindType type, std::shared_ptr<Bind::Bindable> elem)
	{
		switch (type)
		{
		case Draw::BindType::IndexBuffer:
			
			break;
		case Draw::BindType::VertexBuffer:
			vertexBuffer_ptr = std::dynamic_pointer_cast< Bind::VertexBuffer >(elem);
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
		default:
			break;
		}
	}
	void Drawable::CompilePipeline()
	{
		
		Graphics::DescriptorSet::descriptor_set_request_t set_request = {};
		
		set_request.bindings = pipelineLayout_ptr->layout_bindinds;
		set_request.min_descriptor_count = 1;
		set_request.binding_count = pipelineLayout_ptr->dstBinding;
		set_request.stage_flags = VK_SHADER_STAGE_ALL;

		pipeline_ptr->create_descriptor_sets(&pipeline_ptr->pipeline, &set_request, 1);

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		auto bindingDescription = vertexBuffer_ptr->bindingDescription;
		auto attributeDescriptions = vertexBuffer_ptr->attributeDescriptions;

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		vector<VkPipelineShaderStageCreateInfo> stages;

		VkPipelineShaderStageCreateInfo vertex_shader_stage = {};
		vertex_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertex_shader_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertex_shader_stage.module = vertexShader_ptr->shader.module;
		vertex_shader_stage.pName = "main";

		VkPipelineShaderStageCreateInfo frag_shader_stage = {};
		frag_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage.module = pixelShader_ptr->shader.module;
		frag_shader_stage.pName = "main";

		stages.emplace_back(std::move(vertex_shader_stage));
		stages.emplace_back(std::move(frag_shader_stage));

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)vulkan_ptr->GetSwapchain().extent.width;
		viewport.height = (float)vulkan_ptr->GetSwapchain().extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = { .extent = vulkan_ptr->GetSwapchain().extent };

		VkPipelineViewportStateCreateInfo viewport_info = {};
		viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_info.viewportCount = 1;
		viewport_info.scissorCount = 1;
		viewport_info.pScissors = &scissor;
		viewport_info.pViewports = &viewport;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {};
		depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_info.depthTestEnable = VK_TRUE;
		depth_stencil_info.depthWriteEnable = VK_TRUE;
		depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = stages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewport_info;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = &depth_stencil_info;
		
		pipelineInfo.layout = pipeline_ptr->pipeline.pipeline_layout;
		pipelineInfo.renderPass = renderpass_ptr->renderpass.render_pass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		
		if (vkCreateGraphicsPipelines(vulkan_ptr->device.device, NULL, 1, &pipelineInfo, NULL, &pipeline_ptr->pipeline.pipeline)) {
			printf("Failed to create a graphics pipeline for the geometry pass.\n");
			// destroy_point_light_pass(pass, device);
			exit(1);
		}
	}
}