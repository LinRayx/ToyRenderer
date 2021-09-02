#include "Drawable/SSAOgenerateMaterial.h"

namespace Draw
{
	SSAOgenerateMaterial::SSAOgenerateMaterial()
		: MaterialBaseParent()
	{
		using namespace Graphics;
		desc_ptr = make_unique<DescriptorSetCore>();
		Dcb::RawLayout ssaoLayout;
		ssaoLayout.Add<Dcb::Array>("samples").Set<Dcb::Float4>(Gloable::SSAO::SSAO_KERNEL_SIZE);
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

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = initializers::pipelineCreateInfo(desc_ptr->GetPipelineLayout(), nameToRenderPass[RenderPassType::FULLSCREEN]->renderPass, 0);
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewport_info;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();

		vkCreateGraphicsPipelines(Vulkan::getInstance()->GetDevice().device, NULL, 1, &pipelineCreateInfo, nullptr, &pipeline);
	}
}