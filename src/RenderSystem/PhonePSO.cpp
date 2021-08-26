#include "RenderSystem/PhonePSO.h"
#include <stdexcept>

namespace RenderSystem
{
	PhonePSO::PhonePSO() : PipelineStateObject()
	{	
		using namespace Graphics;
		vShader_ptr = make_shared<Bind::VertexShader>("../src/shaders/Phone.vert.glsl", "../src/shaders", "main");
		pShader_ptr = make_shared<Bind::PixelShader>("../src/shaders/Phone.frag.glsl", "../src/shaders", "main");

		desc_layout_ptr = make_shared<DescriptorSetLayout>();

		desc_layout_ptr->Add(LayoutType::SCENE, DescriptorType::UNIFORM, StageFlag::VERTEX);
		desc_layout_ptr->Add(LayoutType::SCENE, DescriptorType::UNIFORM, StageFlag::FRAGMENT);
		desc_layout_ptr->Add(LayoutType::MODEL, DescriptorType::UNIFORM, StageFlag::VERTEX);
		desc_layout_ptr->Add(LayoutType::MODEL, DescriptorType::UNIFORM, StageFlag::FRAGMENT);
		desc_layout_ptr->Add(LayoutType::MODEL, DescriptorType::TEXTURE2D, StageFlag::FRAGMENT);
		desc_layout_ptr->Add(LayoutType::MODEL, DescriptorType::TEXTURE2D, StageFlag::FRAGMENT);
		desc_layout_ptr->Compile();
	}

	PhonePSO::~PhonePSO()
	{
		for (size_t i = 0; i < models.size(); ++i) {
			if (models[i] != nullptr) {
				free(models[i]);
			}
		}
	}

	void PhonePSO::BuildPipeline()
	{
		for (auto& item : drawItems) {
			buildPipeline(item);
		}
	}

	void PhonePSO::BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd)
	{
		auto& drawCmdBuffers = cmd->drawCmdBuffers;
		for (size_t i = 0; i < drawCmdBuffers.size(); i++) {

			auto& rp = Graphics::nameToRenderPass["default"];
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = rp->renderPass;
			renderPassInfo.framebuffer = rp->framebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = Graphics::Vulkan::getInstance()->GetSwapchain().extent;
			renderPassInfo.clearValueCount = static_cast<uint32_t>(rp->clearValues.size());
			renderPassInfo.pClearValues = rp->clearValues.data();

			vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

			for (size_t modelIndex = 0; modelIndex < models.size(); ++modelIndex) {
				for (size_t itemIndex = 0; itemIndex < models[modelIndex]->items.size(); ++itemIndex) {
					VkBuffer vertexBuffers[] = { models[modelIndex]->items[itemIndex].mesh.vertex_buffer->Get() };
					auto indexBuffer = models[modelIndex]->items[itemIndex].mesh.index_buffer;
					VkDeviceSize offsets[] = { 0 };
					auto& material = models[modelIndex]->items[itemIndex].material;

					vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, vertexBuffers, offsets);
					vkCmdBindIndexBuffer(drawCmdBuffers[i], indexBuffer->buffer_ptr->buffers[0], 0, VK_INDEX_TYPE_UINT16);
					vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, desc_layout_ptr->pipelineLayout, 0,
						static_cast<uint32_t>(material.desc_ptr->descriptorSets[i].size()), material.desc_ptr->descriptorSets[i].data(), 0, nullptr);
					//vkCmdDraw(drawCmdBuffers[i], static_cast<uint32_t>(models[modelIndex]->items[itemIndex].mesh.vertex_buffer->buffer_ptr->elem_count), 1, 0, 0);
					vkCmdDrawIndexed(drawCmdBuffers[i], static_cast<uint32_t>(indexBuffer->GetCount()), 1, 0, 0, 0);
				}
			}

			vkCmdEndRenderPass(drawCmdBuffers[i]);
		}
	}

	void PhonePSO::Update(int cur)
	{
		for (size_t i = 0; i < models.size(); ++i) {
			models[i]->Update(cur);
		}
	}

	void PhonePSO::Add(Draw::Model* model)
	{
		models.push_back(model);
		for (size_t i = 0; i < model->items.size(); ++i) {
			drawItems.emplace_back(&model->items[i]);
		}
	}
	std::vector<Draw::Model*>& PhonePSO::GetModels()
	{
		return models;
	}
	void PhonePSO::buildPipeline(Draw::DrawItem* item)
	{
		vBuffer_ptr = item->mesh.vertex_buffer;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vBuffer_ptr->attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &vBuffer_ptr->bindingDescription;;
		vertexInputInfo.pVertexAttributeDescriptions = vBuffer_ptr->attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		vector<VkPipelineShaderStageCreateInfo> stages;

		VkPipelineShaderStageCreateInfo vertex_shader_stage = {};
		vertex_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertex_shader_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertex_shader_stage.module = vShader_ptr->shader.module;
		vertex_shader_stage.pName = "main";

		VkPipelineShaderStageCreateInfo frag_shader_stage = {};
		frag_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage.module = pShader_ptr->shader.module;
		frag_shader_stage.pName = "main";

		stages.emplace_back(std::move(vertex_shader_stage));
		stages.emplace_back(std::move(frag_shader_stage));

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)Graphics::Vulkan::getInstance()->GetSwapchain().extent.width;
		viewport.height = (float)Graphics::Vulkan::getInstance()->GetSwapchain().extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;


		VkRect2D scissor = { .extent = Graphics::Vulkan::getInstance()->GetSwapchain().extent };

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

		vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(1);
		for (size_t i = 0; i < colorBlendAttachments.size(); ++i) {
			colorBlendAttachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachments[i].blendEnable = VK_FALSE;
		}


		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
		colorBlending.pAttachments = colorBlendAttachments.data();
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
		auto depthStencilState = Bind::depthStencilState_ptr->GetDepthStencilState(item->material.depthStencilType);
		pipelineInfo.pDepthStencilState = &depthStencilState;

		pipelineInfo.layout = desc_layout_ptr->pipelineLayout;
		pipelineInfo.renderPass = Graphics::nameToRenderPass["default"]->renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(Graphics::Vulkan::getInstance()->GetDevice().device, NULL, 1, &pipelineInfo, NULL, &pipeline)) {
			throw std::runtime_error("Failed to create a graphics pipeline for the geometry pass.\n");
		}

		item->material.Compile(desc_layout_ptr);
	}
}

