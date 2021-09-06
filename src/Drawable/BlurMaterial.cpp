#include "Drawable/BlurMaterial.h"

namespace Draw
{
	BlurMaterial::BlurMaterial() : MaterialBaseParent()
	{
		desc_ptr = make_shared<Graphics::DescriptorSetCore>();

	}

	void BlurMaterial::Compile()
	{
		cout << "BlurMaterial::Compile()" << endl;
		using namespace Graphics;
		desc_ptr->Compile();
		loadShader(Bind::ShaderType::BLUR);

		VkPipelineVertexInputStateCreateInfo emptyVertexInputState = initializers::pipelineVertexInputStateCreateInfo();
		rasterizationState.cullMode = VK_CULL_MODE_NONE;

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = initializers::pipelineCreateInfo(
			desc_ptr->GetPipelineLayout(), 
			nameToRenderPass[RenderPassType::FULLSCREEN_BLUR]->renderPass, 0);
		pipelineCreateInfo.pVertexInputState = &emptyVertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewport_info;
		depthStencilState.depthTestEnable = VK_FALSE;
		depthStencilState.depthWriteEnable = VK_FALSE;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();

		vkCreateGraphicsPipelines(Vulkan::getInstance()->GetDevice().device, NULL, 1, &pipelineCreateInfo, nullptr, &pipeline);
	}
	void BlurMaterial::BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd)
	{
		auto& drawCmdBuffers = cmd->drawCmdBuffers;

		auto& rp = Graphics::nameToRenderPass[Graphics::RenderPassType::FULLSCREEN_BLUR];

		VkRenderPassBeginInfo renderPassBeginInfo = Graphics::initializers::renderPassBeginInfo();
		renderPassBeginInfo.framebuffer = rp->framebuffer;
		renderPassBeginInfo.renderPass = rp->renderPass;
		renderPassBeginInfo.renderArea.extent.width = rp->width;
		renderPassBeginInfo.renderArea.extent.height = rp->height;
		renderPassBeginInfo.clearValueCount = rp->clearValues.size();
		renderPassBeginInfo.pClearValues = rp->clearValues.data();

		for (size_t i = 0; i < drawCmdBuffers.size(); i++) {
			vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, desc_ptr->GetPipelineLayout(), 0,
				static_cast<uint32_t>(desc_ptr->descriptorSets[i].size()), desc_ptr->descriptorSets[i].data(), 0, nullptr);
			vkCmdDraw(drawCmdBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(drawCmdBuffers[i]);
		}
	}
	void BlurMaterial::AddBlurMap(string name)
	{
		addTexture(Graphics::LayoutType::SCENE, Graphics::StageFlag::FRAGMENT,
			Draw::textureManager->nameToTex[name].textureImageView,
			Draw::textureManager->nameToTex[name].textureSampler);
	}
}