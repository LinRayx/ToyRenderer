#include "Drawable/DeferredMaterial.h"

namespace Draw
{
	DeferredMaterial::DeferredMaterial()
		: PBRMaterial(true)
	{
		using namespace Graphics;
		Dcb::RawLayout layout;

		layout.Add<Dcb::Float>("nearPlane");
		layout.Add<Dcb::Float>("farPlane");
		addLayout("CameraParam", std::move(layout), LayoutType::SCENE, DescriptorType::UNIFORM, StageFlag::FRAGMENT);
		
		matType = MaterialType::GBuffer;
	}

	void DeferredMaterial::Compile()
	{
		cout << "DeferredMaterial::Compile()" << endl;
		desc_ptr->Compile();
		using namespace Graphics;
		VkGraphicsPipelineCreateInfo pipelineCI = initializers::pipelineCreateInfo(desc_ptr->GetPipelineLayout(), nameToRenderPass[RenderPassType::DEFERRED]->renderPass);
		auto& rp = nameToRenderPass[RenderPassType::DEFERRED];
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
		auto viewport = initializers::viewportOffscreen(static_cast<float>(Vulkan::getInstance()->GetWidth()), static_cast<float>(Vulkan::getInstance()->GetHeight()), 0.0, 1.0);
		viewport_info.pViewports = &viewport;

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
	void DeferredMaterial::UpdateSceneData()
	{
		SetValue("CameraParam", "nearPlane", Control::Scene::getInstance()->camera_ptr->GetNearPlane());
		SetValue("CameraParam", "farPlane", Control::Scene::getInstance()->camera_ptr->GetFarPlane());
		SetValue("ViewAndProj", "viewMat", Control::Scene::getInstance()->camera_ptr->GetViewMatrix());
		SetValue("ViewAndProj", "projMat", Control::Scene::getInstance()->camera_ptr->GetProjectMatrix());
	}

}