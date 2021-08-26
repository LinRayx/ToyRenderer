#include "RenderSystem/OutlinePSO.h"

namespace RenderSystem
{
	OutlinePSO::OutlinePSO()
		: PipelineStateObject()
	{
		using namespace Graphics;
		vShader_ptr = make_shared<Bind::VertexShader>("../src/shaders/Stencil.vert.glsl", "../src/shaders", "main");
		pShader_ptr = make_shared<Bind::PixelShader>("../src/shaders/Stencil.frag.glsl", "../src/shaders", "main");
		v_outlineShader_ptr = make_shared<Bind::VertexShader>( "../src/shaders/Outline.vert.glsl", "../src/shaders", "main");
		p_outlineShader_ptr = make_shared<Bind::PixelShader>( "../src/shaders/Outline.frag.glsl", "../src/shaders", "main");
		desc_layout_ptr = make_shared<DescriptorSetLayout>();

		desc_layout_ptr->Add(LayoutType::SCENE, DescriptorType::UNIFORM, StageFlag::VERTEX);
		desc_layout_ptr->Add(LayoutType::SCENE, DescriptorType::UNIFORM, StageFlag::FRAGMENT);
		desc_layout_ptr->Add(LayoutType::MODEL, DescriptorType::UNIFORM, StageFlag::VERTEX);
		desc_layout_ptr->Add(LayoutType::MODEL, DescriptorType::TEXTURE2D, StageFlag::FRAGMENT);
		desc_layout_ptr->Compile();
	}
	void OutlinePSO::BuildPipeline()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = Graphics::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationState = Graphics::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_CLOCKWISE, 0);
		VkPipelineColorBlendAttachmentState blendAttachmentState = Graphics::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
		VkPipelineColorBlendStateCreateInfo colorBlendState = Graphics::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
		VkPipelineDepthStencilStateCreateInfo depthStencilState = Graphics::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewportState = Graphics::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
		VkPipelineMultisampleStateCreateInfo multisampleState = Graphics::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState = Graphics::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

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

		vBuffer_ptr = models[0]->items[0].mesh.vertex_buffer;
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

		VkGraphicsPipelineCreateInfo pipelineCI = Graphics::initializers::pipelineCreateInfo(desc_layout_ptr->pipelineLayout, Graphics::nameToRenderPass["default"]->renderPass, 0);
		pipelineCI.pInputAssemblyState = &inputAssemblyState;
		pipelineCI.pRasterizationState = &rasterizationState;
		pipelineCI.pColorBlendState = &colorBlendState;
		pipelineCI.pMultisampleState = &multisampleState;
		pipelineCI.pViewportState = &viewportState;
		pipelineCI.pDepthStencilState = &depthStencilState;
		pipelineCI.pDynamicState = &dynamicState;
		pipelineCI.stageCount = static_cast<uint32_t>(stages.size());
		pipelineCI.pStages = stages.data();
		pipelineCI.pVertexInputState = &vertexInputInfo;

		// stencil pipeline
		// 1. stencil test
		// 2, depth test
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		depthStencilState.stencilTestEnable = VK_TRUE;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;	// always pass stencil test
		depthStencilState.back.failOp = VK_STENCIL_OP_REPLACE;		// failed stencil test
		depthStencilState.back.depthFailOp = VK_STENCIL_OP_REPLACE; // pass stencil test but failed depth test
		depthStencilState.back.passOp = VK_STENCIL_OP_REPLACE;		// pass both the depth and stencil tests.
		depthStencilState.back.compareMask = 0xff;
		depthStencilState.back.writeMask = 0xff;
		depthStencilState.back.reference = 1;
		depthStencilState.front = depthStencilState.back;

		vkCreateGraphicsPipelines(Graphics::Vulkan::getInstance()->GetDevice().device, nullptr, 1, &pipelineCI, nullptr, &pipelineStencil);

		// outline pipeline
		depthStencilState.back.compareOp = VK_COMPARE_OP_NOT_EQUAL;	// only pass stencil test when A != B
		depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;			
		depthStencilState.back.depthFailOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.passOp = VK_STENCIL_OP_REPLACE;
		depthStencilState.front = depthStencilState.back;
		depthStencilState.depthTestEnable = VK_FALSE;

		stages[0].module = v_outlineShader_ptr->shader.module;
		stages[1].module = p_outlineShader_ptr->shader.module;

		vkCreateGraphicsPipelines(Graphics::Vulkan::getInstance()->GetDevice().device, nullptr, 1, &pipelineCI, nullptr, &pipelineOutline);
	}

	void OutlinePSO::BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd)
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
					vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineStencil);
					vkCmdDrawIndexed(drawCmdBuffers[i], static_cast<uint32_t>(indexBuffer->GetCount()), 1, 0, 0, 0);
					vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineOutline);
					vkCmdDrawIndexed(drawCmdBuffers[i], static_cast<uint32_t>(indexBuffer->GetCount()), 1, 0, 0, 0);
				}
			}

			vkCmdEndRenderPass(drawCmdBuffers[i]);
		}
	}
	void OutlinePSO::Update(int cur)
	{
	}
	void OutlinePSO::Add(Draw::Model* model)
	{
	}
}