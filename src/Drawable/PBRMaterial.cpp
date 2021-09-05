#include "Drawable/PBRMaterial.h"

namespace Draw
{
	PBRMaterial::PBRMaterial(bool flag) :MaterialBase(true)
	{
		InitPBRData();
	}
	PBRMaterial::PBRMaterial()
		:MaterialBase(true)
	{

		Dcb::RawLayout layout2;
		layout2.Add<Dcb::Float3>("viewPos");
		layout2.Add<Dcb::Float3>("direLightDir");
		layout2.Add<Dcb::Float3>("direLightColor");

		addLayout("Light", std::move(layout2), Graphics::LayoutType::SCENE, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::FRAGMENT);
		addTexture(Graphics::LayoutType::SCENE, Graphics::StageFlag::FRAGMENT, Draw::textureManager->nameToTex["brdf_lut"].textureImageView, Draw::textureManager->nameToTex["brdf_lut"].textureSampler);
		addCubeTexture("irradiance_map");
		addCubeTexture("prefilter_map");
		InitPBRData();

		matType = MaterialType::PBR;
	}

	void PBRMaterial::LoadModelTexture(const aiMaterial* material, string directory, string meshName)
	{
		addPBRTexture("AlbedoMap");
		addPBRTexture("MetallicMap");
		addPBRTexture("NormalMap");
		addPBRTexture("RoughnessMap");
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

	bool PBRMaterial::SetUI()
	{
		bool dirty = false;
		const auto dcheck = [&dirty](bool changed) {dirty = dirty || changed; };
		ImGui::TextColored({ 0.4f,1.0f,0.6f,1.0f }, "PBRMaterial");
		ImVec4 albedoUI = ImVec4(albedo.x, albedo.y, albedo.z, 1.00f);
		dcheck(ImGui::ColorEdit3("albedo", (float*)&albedoUI));
		dcheck(ImGui::SliderFloat("metallic", &metallic, 0, 1.f));
		dcheck(ImGui::SliderFloat("roughness", &roughness, 0, 1.f));
		this->albedo = glm::vec3(albedoUI.x, albedoUI.y, albedoUI.z);
		SetValue("PbrParam", "metallic", metallic);
		SetValue("PbrParam", "roughness", roughness);
		SetValue("PbrParam", "albedo", albedo);
		return dirty;
	}

	float& PBRMaterial::GetMetallic()
	{
		return metallic;
	}

	glm::vec3& PBRMaterial::GetAlbedo()
	{
		return albedo;
	}

	float& PBRMaterial::GetRoughness()
	{
		return roughness;
	}

	void PBRMaterial::InitPBRData()
	{

		using namespace Graphics;
		Dcb::RawLayout pbrLayout;
		pbrLayout.Add<Dcb::Bool>("HasAlbedoMap");
		pbrLayout.Add<Dcb::Bool>("HasMetallicMap");
		pbrLayout.Add<Dcb::Bool>("HasNormalMap");
		pbrLayout.Add<Dcb::Bool>("HasRoughnessMap");
		pbrLayout.Add<Dcb::Float3>("albedo");
		pbrLayout.Add<Dcb::Float>("metallic");
		pbrLayout.Add<Dcb::Float>("roughness");

		addLayout("PbrParam", std::move(pbrLayout), LayoutType::PBRTEXTURE, DescriptorType::UNIFORM, StageFlag::FRAGMENT);




		SetValue("PbrParam", "metallic", metallic);
		SetValue("PbrParam", "roughness", roughness);
		SetValue("PbrParam", "albedo", albedo);
	}

	void PBRMaterial::addPBRTexture(string name)
	{
		using namespace Graphics;
		string texName = modelName + "_" + name;
		if (Draw::textureManager->nameToTex.count(texName) > 0) {
			addTexture(LayoutType::PBRTEXTURE, StageFlag::FRAGMENT, Draw::textureManager->nameToTex[texName].textureImageView, Draw::textureManager->nameToTex[texName].textureSampler,
				pbrbinding++);
			SetValue("PbrParam", "Has" + name , true);
		}
		else {
			//addTexture(LayoutType::PBRTEXTURE, StageFlag::FRAGMENT, VK_NULL_HANDLE, VK_NULL_HANDLE,
			//	pbrbinding++);
				addTexture(LayoutType::PBRTEXTURE, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["ssaoNoiseMap"].textureImageView, 
					Draw::textureManager->nameToTex["ssaoNoiseMap"].textureSampler,
					pbrbinding++);
			SetValue("PbrParam", "Has" + name, false);
		}
	}

}