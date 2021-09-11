#include "Drawable/SSAOgenerateMaterial.h"

namespace Draw
{
	SSAOgenerateMaterial::SSAOgenerateMaterial()
		: MaterialBaseParent()
	{
		using namespace Graphics;
		desc_ptr = make_unique<DescriptorSetCore>();
		Dcb::RawLayout sceneLayout;
		sceneLayout.Add<Dcb::Matrix>("view");
		sceneLayout.Add<Dcb::Matrix>("proj");
		addLayout("Scene", std::move(sceneLayout), LayoutType::SCENE, DescriptorType::UNIFORM, StageFlag::FRAGMENT);
	
		Dcb::RawLayout ssaoLayout;
		ssaoLayout.Add<Dcb::Array>("samples");
		ssaoLayout["samples"].Set<Dcb::Float4>(Gloable::SSAO::SSAO_KERNEL_SIZE);
		addLayout("SSAOKernel", std::move(ssaoLayout), LayoutType::SCENE, DescriptorType::UNIFORM, StageFlag::FRAGMENT);
		for (size_t i = 0; i < Gloable::SSAO::SSAO_KERNEL_SIZE; ++i) {
			SetValue("SSAOKernel", "samples", i, Gloable::SSAO::ssaoKernel[i]);
		}
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex["GBuffer_position"].textureImageView, textureManager->nameToTex["GBuffer_position"].textureSampler);
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex["GBuffer_normals"].textureImageView, textureManager->nameToTex["GBuffer_normals"].textureSampler);
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex["ssaoNoiseMap"].textureImageView, textureManager->nameToTex["ssaoNoiseMap"].textureSampler);
	}
	void SSAOgenerateMaterial::Compile()
	{
		cout << "SSAOgenerateMaterial::Compile()" << endl;
		using namespace Graphics;
		desc_ptr->Compile();

		// SSAO Kernel size and radius are constant for this pipeline, so we set them using specialization constants
		struct SpecializationData {
			uint32_t kernelSize = Gloable::SSAO::SSAO_KERNEL_SIZE;
			float radius = Gloable::SSAO::SSAO_RADIUS;
		} specializationData;
		std::array<VkSpecializationMapEntry, 2> specializationMapEntries = {
			initializers::specializationMapEntry(0, offsetof(SpecializationData, kernelSize), sizeof(SpecializationData::kernelSize)),
			initializers::specializationMapEntry(1, offsetof(SpecializationData, radius), sizeof(SpecializationData::radius))
		};
		VkSpecializationInfo specializationInfo = initializers::specializationInfo(2, specializationMapEntries.data(), sizeof(specializationData), &specializationData);
		loadShader(Bind::ShaderType::SSAO, Bind::ShaderType::SSAO, specializationInfo);

		VkPipelineVertexInputStateCreateInfo emptyVertexInputState = initializers::pipelineVertexInputStateCreateInfo();
		rasterizationState.cullMode = VK_CULL_MODE_NONE;

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = initializers::pipelineCreateInfo(desc_ptr->GetPipelineLayout(), nameToRenderPass[RenderPassType::FULLSCREEN_SSAO]->renderPass, 0);
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
	void SSAOgenerateMaterial::BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd)
	{
		auto& drawCmdBuffers = cmd->drawCmdBuffers;
		auto& rp = Graphics::nameToRenderPass[Graphics::RenderPassType::FULLSCREEN_SSAO];
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

	void SSAOgenerateMaterial::UpdateSceneData()
	{
		SetValue("Scene", "view", Control::Scene::getInstance()->camera_ptr->GetViewMatrix());
		SetValue("Scene", "proj", Control::Scene::getInstance()->camera_ptr->GetProjectMatrix());
	}
}