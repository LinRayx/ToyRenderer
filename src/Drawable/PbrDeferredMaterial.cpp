#include "Drawable/PbrDeferredMaterial.h"

namespace Draw
{
	/// <summary>
	/// layout(set = 0, binding = 2) uniform sampler2D brdfLUT;
	// layout(set = 0, binding = 3) uniform samplerCube irradianceMap;
	// layout(set = 0, binding = 4) uniform samplerCube prefilteredMap;
	// layout(set = 0, binding = 5) uniform sampler2D gbuffer_positionDepthMap;
	// layout(set = 0, binding = 6) uniform sampler2D gbuffer_normalMap;
	// layout(set = 0, binding = 7) uniform sampler2D gbuffer_albedoMap;
	// layout(set = 0, binding = 8) uniform sampler2D gbuffer_metallicRoughnessMap;
	// layout(set = 0, binding = 9) uniform sampler2D ssaoMap;
	/// </summary>
	PbrDeferredMaterial::PbrDeferredMaterial() : MaterialBaseParent()
	{
		desc_ptr = std::make_shared<Graphics::DescriptorSetCore>();

		using namespace Graphics;
		Dcb::RawLayout layout2;
		layout2.Add<Dcb::Float3>("viewPos");
		layout2.Add<Dcb::Float3>("direLightDir");
		layout2.Add<Dcb::Float3>("direLightColor");

		addLayout("Light", std::move(layout2), LayoutType::SCENE, DescriptorType::UNIFORM, StageFlag::FRAGMENT);
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["brdf_lut"].textureImageView, Draw::textureManager->nameToTex["brdf_lut"].textureSampler);
		addCubeTexture("irradiance_map");
		addCubeTexture("prefilter_map");
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex["GBuffer_position"].textureImageView, textureManager->nameToTex["GBuffer_position"].textureSampler);
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex["GBuffer_normals"].textureImageView, textureManager->nameToTex["GBuffer_normals"].textureSampler);
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex["GBuffer_albedo"].textureImageView, textureManager->nameToTex["GBuffer_albedo"].textureSampler);
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex["GBuffer_metallic_roughness"].textureImageView, textureManager->nameToTex["GBuffer_metallic_roughness"].textureSampler);
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex["ssaoMap"].textureImageView, textureManager->nameToTex["ssaoMap"].textureSampler);
	}
	void PbrDeferredMaterial::Compile()
	{
		cout << "PbrDeferredMaterial::Compile()" << endl;
		using namespace Graphics;
		desc_ptr->Compile();

		loadShader(Bind::ShaderType::PBR_Deferred);

		VkPipelineVertexInputStateCreateInfo emptyVertexInputState = initializers::pipelineVertexInputStateCreateInfo();
		rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = initializers::pipelineCreateInfo(desc_ptr->GetPipelineLayout(), nameToRenderPass[RenderPassType::Default]->renderPass, 0);
		pipelineCreateInfo.pVertexInputState = &emptyVertexInputState;
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

	void PbrDeferredMaterial::UpdateSceneData()
	{
		SetValue("Light", "viewPos", Control::Scene::getInstance()->camera_ptr->GetViewPos());
		SetValue("Light", "direLightDir", Control::Scene::getInstance()->directionLight.direciton);
		SetValue("Light", "direLightColor", Control::Scene::getInstance()->directionLight.color);
	}
}