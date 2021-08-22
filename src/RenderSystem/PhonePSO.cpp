#include "RenderSystem/PhonePSO.h"
#include <stdexcept>

namespace RenderSystem
{
	PhonePSO::PhonePSO(shared_ptr<Graphics::Vulkan> vulkan_ptr, shared_ptr<Graphics::DescriptorPool> desc_pool_ptr, shared_ptr<Graphics::Image> image_ptr) : PipelineStateObject(vulkan_ptr)
	{
		vShader_ptr = make_shared<Bind::VertexShader>(vulkan_ptr, "../src/shaders/Phone.vert.glsl", "../src/shaders", "main");
		pShader_ptr = make_shared<Bind::PixelShader>(vulkan_ptr, "../src/shaders/Phone.frag.glsl", "../src/shaders", "main");
		pipeline_ptr = make_shared<Graphics::Pipeline>(vulkan_ptr);
		desc_layout_ptr = make_shared<Graphics::DescriptorSetLayout>(vulkan_ptr);

		desc_layout_ptr->Add(Graphics::LayoutType::SCENE, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::VERTEX);
		desc_layout_ptr->Add(Graphics::LayoutType::SCENE, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::FRAGMENT);
		desc_layout_ptr->Add(Graphics::LayoutType::MODEL, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::VERTEX);
		desc_layout_ptr->Add(Graphics::LayoutType::MODEL, Graphics::DescriptorType::TEXTURE2D, Graphics::StageFlag::FRAGMENT);

		using namespace Dcb;
		Dcb::VertexBuffer vbuf(
			std::move(
				Dcb::VertexLayout{}
				.Append(VertexLayout::Position3D)
				.Append(VertexLayout::Normal)
			)
		);

		renderpass_ptr = make_shared<Graphics::RenderPass>(vulkan_ptr, image_ptr);

		//renderpass_ptr->AddResource("Color1", false);
		renderpass_ptr->AddResource("Depth", true);

		renderpass_ptr->CreateRenderPass();
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
		vBuffer_ptr = models[0]->items[0].mesh.vertex_buffer;
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		auto bindingDescription = vBuffer_ptr->bindingDescription;
		auto attributeDescriptions = vBuffer_ptr->attributeDescriptions;

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
		pipelineInfo.pDepthStencilState = &depth_stencil_info;

		desc_layout_ptr->Compile();
		pipelineInfo.layout = desc_layout_ptr->pipelineLayout;
		pipelineInfo.renderPass = renderpass_ptr->renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(vulkan_ptr->GetDevice().device, NULL, 1, &pipelineInfo, NULL, &pipeline_ptr->pipeline)) {
			throw std::runtime_error("Failed to create a graphics pipeline for the geometry pass.\n");
		}

		for (size_t i = 0; i < models.size(); ++i) {
			models[i]->BuildDesc(desc_layout_ptr);
		}
	}

	void PhonePSO::BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd)
	{
		auto& drawCmdBuffers = cmd->drawCmdBuffers;
		for (size_t i = 0; i < drawCmdBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(drawCmdBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderpass_ptr->renderPass;
			renderPassInfo.framebuffer = renderpass_ptr->framebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = vulkan_ptr->GetSwapchain().extent;


			renderPassInfo.clearValueCount = static_cast<uint32_t>(renderpass_ptr->clearValues.size());
			renderPassInfo.pClearValues = renderpass_ptr->clearValues.data();

			vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_ptr->pipeline);

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

			if (vkEndCommandBuffer(drawCmdBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
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
	}
}

