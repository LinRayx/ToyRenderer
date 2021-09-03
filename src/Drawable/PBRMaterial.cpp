#include "Drawable/PBRMaterial.h"

namespace Draw
{
	PBRMaterial::PBRMaterial()
		:MaterialBase(true)
	{

		Dcb::RawLayout layout2;
		layout2.Add<Dcb::Float3>("viewPos");
		layout2.Add<Dcb::Float3>("direLightDir");
		layout2.Add<Dcb::Float3>("direLightColor");

		addLayout("Light", std::move(layout2), Graphics::LayoutType::SCENE, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::FRAGMENT);

		using namespace Graphics;

		Dcb::RawLayout pbrLayout;
		pbrLayout.Add<Dcb::Float>("metallic");
		pbrLayout.Add<Dcb::Float>("roughness");
		pbrLayout.Add<Dcb::Bool>("HasDiffuseTex");

		addLayout("PbrParam", std::move(pbrLayout), LayoutType::MODEL, DescriptorType::UNIFORM, StageFlag::FRAGMENT);
		
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["brdf_lut"].textureImageView, Draw::textureManager->nameToTex["brdf_lut"].textureSampler);
		AddCubeTexture("irradiance_map");
		AddCubeTexture("prefilter_map");
		addTexture(LayoutType::MODEL, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["albedoMap"].textureImageView, Draw::textureManager->nameToTex["albedoMap"].textureSampler);
		addTexture(LayoutType::MODEL, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["metallicMap"].textureImageView, Draw::textureManager->nameToTex["metallicMap"].textureSampler);
		addTexture(LayoutType::MODEL, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["normalMap"].textureImageView, Draw::textureManager->nameToTex["normalMap"].textureSampler);
		addTexture(LayoutType::MODEL, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["roughnessMap"].textureImageView, Draw::textureManager->nameToTex["roughnessMap"].textureSampler);
		addTexture(LayoutType::MODEL, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["aoMap"].textureImageView, Draw::textureManager->nameToTex["aoMap"].textureSampler);

		SetValue("PbrParam", "metallic", 1.f);
		SetValue("PbrParam", "roughness", 1.0f);
		matType = MaterialType::PBR;
	}

	void PBRMaterial::LoadModelTexture(const aiMaterial* material, string directory, string meshName)
	{
		
	}

	void PBRMaterial::Compile()
	{
		cout << "PBRMaterial::Compile()" << endl;
		desc_ptr->Compile();
		using namespace Graphics;
		VkGraphicsPipelineCreateInfo pipelineCI = initializers::pipelineCreateInfo(desc_ptr->GetPipelineLayout(), nameToRenderPass[RenderPassType::Default]->renderPass);
		
		loadVertexInfo();
		loadShader(Bind::ShaderType::PBR);
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

	void PBRMaterial::UpdateSceneData()
	{
		MaterialBase::UpdateSceneData();
		SetValue("Light", "viewPos", Control::Scene::getInstance()->camera_ptr->GetViewPos());
		SetValue("Light", "direLightDir", Control::Scene::getInstance()->directionLight.direciton);
		SetValue("Light", "direLightColor", Control::Scene::getInstance()->directionLight.color);
	}

	void PBRMaterial::AddCubeTexture(string cube_texture_name)
	{
		using namespace Graphics;
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex[cube_texture_name].textureImageView,
			textureManager->nameToTex[cube_texture_name].textureSampler);
	}

	
}