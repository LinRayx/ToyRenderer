#include "Drawable/SkyboxMaterial.h"

namespace Draw
{
	SkyboxMaterial::SkyboxMaterial() : MaterialBase(true)
	{
		using namespace Graphics;
		AddCubeTexture("skybox_texture");
		matType = MaterialType::Skybox;
	}
	void SkyboxMaterial::AddCubeTexture(string skybox_texture_name)
	{
		using namespace Graphics;
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex[skybox_texture_name].textureImageView,
			textureManager->nameToTex[skybox_texture_name].textureSampler);
	}
	void SkyboxMaterial::Compile()
	{
		desc_ptr->Compile();
		
		loadShader(Bind::ShaderType::Skybox);
		// Skybox pipeline (background cube)
		rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;

		VkGraphicsPipelineCreateInfo pipelineCI = Graphics::initializers::pipelineCreateInfo(desc_ptr->GetPipelineLayout(), Graphics::nameToRenderPass[Graphics::RenderPassType::Default]->renderPass, 0);
		pipelineCI.pInputAssemblyState = &inputAssemblyState;
		pipelineCI.pRasterizationState = &rasterizationState;
		pipelineCI.pColorBlendState = &colorBlendState;
		pipelineCI.pMultisampleState = &multisampleState;
		pipelineCI.pViewportState = &viewportState;
		auto depthStencilState = Bind::depthStencilState_ptr->GetDepthStencilState(Bind::DepthStencilStateType::Default);
		pipelineCI.pDepthStencilState = &depthStencilState;
		pipelineCI.stageCount = shaderStages.size();
		pipelineCI.pStages = shaderStages.data();
		pipelineCI.pVertexInputState = &vertexInputInfo;
		pipelineCI.pViewportState = &viewport_info;


		vkCreateGraphicsPipelines(Graphics::Vulkan::getInstance()->GetDevice().device, NULL, 1, &pipelineCI, nullptr, &pipeline);
	}
}