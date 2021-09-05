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
		cout << "SkyboxMaterial::Compile()" << endl;
		desc_ptr->Compile();
		
		loadVertexInfo();
		loadShader(Bind::ShaderType::Skybox);
		// Skybox pipeline (background cube)
		rasterizationState.cullMode = VK_CULL_MODE_NONE;

		VkGraphicsPipelineCreateInfo pipelineCI = Graphics::initializers::pipelineCreateInfo(desc_ptr->GetPipelineLayout(), Graphics::nameToRenderPass[Graphics::RenderPassType::Default]->renderPass, 0);
		pipelineCI.pInputAssemblyState = &inputAssemblyState;
		pipelineCI.pRasterizationState = &rasterizationState;
		pipelineCI.pColorBlendState = &colorBlendState;
		pipelineCI.pMultisampleState = &multisampleState;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		pipelineCI.pDepthStencilState = &depthStencilState;
		pipelineCI.stageCount = shaderStages.size();
		pipelineCI.pStages = shaderStages.data();
		pipelineCI.pVertexInputState = &vertexInputInfo;
		pipelineCI.pViewportState = &viewport_info;

		vkCreateGraphicsPipelines(Graphics::Vulkan::getInstance()->GetDevice().device, NULL, 1, &pipelineCI, nullptr, &pipeline);
	}
	void SkyboxMaterial::UpdateSceneData()
	{
		MaterialBase::UpdateSceneData();
		SetValue("ViewAndProj", "viewMat", glm::mat4(glm::mat3(Control::Scene::getInstance()->camera_ptr->GetViewMatrix())));
	}
}