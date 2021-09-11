#include "Drawable/BlurMaterial.h"

namespace Draw
{
	BlurMaterial::BlurMaterial() : MaterialBaseParent()
	{
		desc_ptr = make_shared<Graphics::DescriptorSetCore>();

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
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(rp->clearValues.size());
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
	void BlurMaterial::initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo)
	{
		using namespace Graphics;
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::FULLSCREEN_VERT, VK_SHADER_STAGE_VERTEX_BIT, std::move(vert_defs)));
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::BLUR, VK_SHADER_STAGE_FRAGMENT_BIT, std::move(frag_defs)));
		depthStencilState.depthTestEnable = VK_FALSE;
		depthStencilState.depthWriteEnable = VK_FALSE;
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		pinfo.renderPass = nameToRenderPass[RenderPassType::FULLSCREEN_BLUR]->renderPass;
		pinfo.pVertexInputState = &emptyVertexInputState;
	}
}