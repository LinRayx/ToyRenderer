#include "Drawable/BrdfMaterial.h"

namespace Draw
{
	BrdfMaterial::BrdfMaterial()
	{
		desc_ptr = std::make_unique<Graphics::DescriptorSetCore>();
		desc_ptr->Compile();
	}
	BrdfMaterial::~BrdfMaterial()
	{
		vkDestroyPipeline(Graphics::Vulkan::getInstance()->GetDevice().device, pipeline, Graphics::Vulkan::getInstance()->GetDevice().allocator);
	}
	void BrdfMaterial::Compile()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = Graphics::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationState = Graphics::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
		VkPipelineColorBlendAttachmentState blendAttachmentState = Graphics::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
		VkPipelineColorBlendStateCreateInfo colorBlendState = Graphics::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
		VkPipelineDepthStencilStateCreateInfo depthStencilState = Graphics::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineMultisampleStateCreateInfo multisampleState = Graphics::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
		VkPipelineVertexInputStateCreateInfo emptyInputState = Graphics::initializers::pipelineVertexInputStateCreateInfo();

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

		VkPipelineShaderStageCreateInfo vertex_shader_stage = {};
		vertex_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertex_shader_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertex_shader_stage.module = Bind::shaderFactory[Bind::ShaderType::BRDFLUT]->vert_shader->GetShaderModule();
		vertex_shader_stage.pName = "main";

		VkPipelineShaderStageCreateInfo frag_shader_stage = {};
		frag_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage.module = Bind::shaderFactory[Bind::ShaderType::BRDFLUT]->frag_shader->GetShaderModule();
		frag_shader_stage.pName = "main";

		shaderStages.emplace_back(std::move(vertex_shader_stage));
		shaderStages.emplace_back(std::move(frag_shader_stage));

		VkViewport viewport = Graphics::initializers::viewportOffscreen((float)dim, (float)dim, 0.0f, 1.0f);
		VkRect2D scissor = Graphics::initializers::rect2D(dim, dim, 0, 0);

		VkPipelineViewportStateCreateInfo viewport_info = {};
		viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_info.viewportCount = 1;
		viewport_info.scissorCount = 1;
		viewport_info.pScissors = &scissor;
		viewport_info.pViewports = &viewport;


		VkGraphicsPipelineCreateInfo pipelineCI = Graphics::initializers::pipelineCreateInfo(desc_ptr->GetPipelineLayout(), Graphics::nameToRenderPass[Graphics::RenderPassType::BRDFLUT]->renderPass, 0);
		pipelineCI.pInputAssemblyState = &inputAssemblyState;
		pipelineCI.pRasterizationState = &rasterizationState;
		pipelineCI.pColorBlendState = &colorBlendState;
		pipelineCI.pMultisampleState = &multisampleState;
		pipelineCI.pDepthStencilState = &depthStencilState;
		pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCI.pStages = shaderStages.data();
		pipelineCI.pVertexInputState = &emptyInputState;
		pipelineCI.pViewportState = &viewport_info;

		if (vkCreateGraphicsPipelines(Graphics::Vulkan::getInstance()->GetDevice().device, NULL, 1, &pipelineCI, NULL, &pipeline)) {
			throw std::runtime_error("Failed to create a graphics pipeline for the geometry pass.\n");
		}
	}
	void BrdfMaterial::Execute(shared_ptr<Graphics::CommandBuffer> cmd)
	{
		cout << "BrdfMaterial::Execute" << endl;
		auto cmdBuf = cmd->beginSingleTimeCommands();
		const int32_t dim = 512;
		// Render
		VkClearValue clearValues[1];
		clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

		VkRenderPassBeginInfo renderPassBeginInfo = Graphics::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = Graphics::nameToRenderPass[Graphics::RenderPassType::BRDFLUT]->renderPass;
		renderPassBeginInfo.renderArea.extent.width = dim;
		renderPassBeginInfo.renderArea.extent.height = dim;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = Graphics::nameToRenderPass[Graphics::RenderPassType::BRDFLUT]->framebuffer;
		vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdDraw(cmdBuf, 3, 1, 0, 0);

		vkCmdEndRenderPass(cmdBuf);
		cmd->endSingleTimeCommands(cmdBuf);
	}
}