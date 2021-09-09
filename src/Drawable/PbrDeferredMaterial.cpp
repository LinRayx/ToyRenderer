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

		layout2.Add<Dcb::Array>("PointLights");
		layout2["PointLights"].Set<Dcb::Struct>(Control::Scene::getInstance()->pointLights.size());
		layout2["PointLights"].T().Add<Dcb::Struct>("pl");
		layout2["PointLights"].T()["pl"].Add<Dcb::Float3>("position");
		layout2["PointLights"].T()["pl"].Add<Dcb::Float4>("color");
		layout2["PointLights"].T()["pl"].Add<Dcb::Float>("constant");
		layout2["PointLights"].T()["pl"].Add<Dcb::Float>("linear");
		layout2["PointLights"].T()["pl"].Add<Dcb::Float>("quadratic");

		layout2.Add<Dcb::Bool>("SSAO");

		addLayout("Light", std::move(layout2), LayoutType::SCENE, DescriptorType::UNIFORM, StageFlag::FRAGMENT);
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["brdf_lut"].textureImageView, Draw::textureManager->nameToTex["brdf_lut"].textureSampler);
		addCubeTexture("irradiance_map");
		addCubeTexture("prefilter_map");
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex["GBuffer_position"].textureImageView, textureManager->nameToTex["GBuffer_position"].textureSampler);
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex["GBuffer_normals"].textureImageView, textureManager->nameToTex["GBuffer_normals"].textureSampler);
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex["GBuffer_albedo"].textureImageView, textureManager->nameToTex["GBuffer_albedo"].textureSampler);
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex["GBuffer_metallic_roughness"].textureImageView, textureManager->nameToTex["GBuffer_metallic_roughness"].textureSampler);
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex["ssaoBlurMap"].textureImageView, textureManager->nameToTex["ssaoMap"].textureSampler);
		addCubeTexture("omni_depth_map");
	}
	void PbrDeferredMaterial::Compile()
	{
		cout << "PbrDeferredMaterial::Compile()" << endl;
		using namespace Graphics;
		desc_ptr->Compile();

		loadShader(Bind::ShaderType::PBR_Deferred);

		VkPipelineVertexInputStateCreateInfo emptyVertexInputState = initializers::pipelineVertexInputStateCreateInfo();
		float width = static_cast<float>(Vulkan::getInstance()->GetWidth());
		float height = static_cast<float>(Vulkan::getInstance()->GetHeight());
		VkViewport viewport = initializers::viewport(width, height, 0.0f, 1.0f);
		VkRect2D scissor = initializers::rect2D(width, height, 0, 0);

		VkPipelineViewportStateCreateInfo viewport_info = {};
		viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_info.viewportCount = 1;
		viewport_info.scissorCount = 1;
		viewport_info.pScissors = &scissor;
		viewport_info.pViewports = &viewport;
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		VkGraphicsPipelineCreateInfo pipelineCreateInfo = initializers::pipelineCreateInfo(desc_ptr->GetPipelineLayout(), nameToRenderPass[RenderPassType::Default]->renderPass, 0);
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
		cout << "PbrDeferredMaterial::Compile() End" << endl;
	}

	void PbrDeferredMaterial::UpdateSceneData()
	{
		SetValue("Light", "viewPos", Control::Scene::getInstance()->camera_ptr->GetViewPos());
		SetValue("Light", "SSAO", Control::Scene::getInstance()->SSAO);

		auto& pl = Control::Scene::getInstance()->pointLights;
		string key = "pl";
		for (size_t i = 0; i < pl.size(); ++i) {
			// b[PointLights][pointLight][i][position] = 
			SetValue("Light", "PointLights", key, "position", i,  pl[i].GetLightPosition());
			SetValue("Light", "PointLights", key, "color", i, pl[i].GetLightColor());
			SetValue("Light", "PointLights", key, "constant", i, pl[i].GetLightConstant());
			SetValue("Light", "PointLights", key, "linear", i, pl[i].GetLightLinear());
			SetValue("Light", "PointLights", key, "quadratic", i, pl[i].GetLightQuadratic());
			// std::cout << pl[i].GetLightColor().x << " " << pl[i].GetLightColor().y << " " << pl[i].GetLightColor().z << std::endl;
		}
	}
}