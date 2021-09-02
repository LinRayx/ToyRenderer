#include "Drawable/DeferredMaterial.h"

namespace Draw
{
	DeferredMaterial::DeferredMaterial()
		: MaterialBase()
	{
		using namespace Graphics;
		desc_ptr = std::make_shared<DescriptorSetCore>();
		Dcb::RawLayout layout;
		layout.Add<Dcb::Matrix>("viewMat");
		layout.Add<Dcb::Matrix>("projMat");
		layout.Add<Dcb::Float>("nearPlane");
		layout.Add<Dcb::Float>("farPlane");
		addLayout("ViewAndProj", std::move(layout), LayoutType::SCENE, DescriptorType::UNIFORM, StageFlag::ALL);

		Dcb::RawLayout transBuf;
		transBuf.Add<Dcb::Matrix>("modelTrans");
		addLayout("Model", std::move(transBuf), LayoutType::MODEL, DescriptorType::UNIFORM, StageFlag::ALL);

		addTexture(LayoutType::MODEL, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["albedoMap"].textureImageView, Draw::textureManager->nameToTex["albedoMap"].textureSampler);
		addTexture(LayoutType::MODEL, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["metallicMap"].textureImageView, Draw::textureManager->nameToTex["metallicMap"].textureSampler);
		addTexture(LayoutType::MODEL, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["normalMap"].textureImageView, Draw::textureManager->nameToTex["normalMap"].textureSampler);
		addTexture(LayoutType::MODEL, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["roughnessMap"].textureImageView, Draw::textureManager->nameToTex["roughnessMap"].textureSampler);
		addTexture(LayoutType::MODEL, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["aoMap"].textureImageView, Draw::textureManager->nameToTex["aoMap"].textureSampler);
		
		matType = MaterialType::GBuffer;
	}

	void DeferredMaterial::Compile()
	{
		cout << "DeferredMaterial::Compile()" << endl;
		desc_ptr->Compile();
		using namespace Graphics;
		VkGraphicsPipelineCreateInfo pipelineCI = initializers::pipelineCreateInfo(desc_ptr->GetPipelineLayout(), nameToRenderPass[RenderPassType::DEFERRED]->renderPass);

		// Blend attachment states required for all color attachments
		// This is important, as color write mask will otherwise be 0x0 and you
		// won't see anything rendered to the attachment
		std::array<VkPipelineColorBlendAttachmentState, 4> blendAttachmentStates = {
			initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
			initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
			initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
			initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE)
		};
		colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
		colorBlendState.pAttachments = blendAttachmentStates.data();
		rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;

		loadVertexInfo();
		loadShader(Bind::ShaderType::GBUFFER);

		pipelineCI.pInputAssemblyState = &inputAssemblyState;
		pipelineCI.pRasterizationState = &rasterizationState;
		pipelineCI.pColorBlendState = &colorBlendState;
		pipelineCI.pMultisampleState = &multisampleState;
		pipelineCI.pViewportState = &viewport_info;
		auto depthStencilState = Bind::depthStencilState_ptr->GetDepthStencilState(Bind::DepthStencilStateType::WriteStencil);

		pipelineCI.pDepthStencilState = &depthStencilState;
		pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCI.pStages = shaderStages.data();

		pipelineCI.pVertexInputState = &vertexInputInfo;

		if (vkCreateGraphicsPipelines(Vulkan::getInstance()->GetDevice().device, NULL, 1, &pipelineCI, NULL, &pipeline)) {
			throw std::runtime_error("Failed to create a graphics pipeline for the geometry pass.\n");
		}
	}
}