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
		addPBRTexture("Albedo");
		addPBRTexture("Metallic");
		addPBRTexture("Normal");
		addPBRTexture("Roughness");
	}

	void PBRMaterial::UpdateSceneData()
	{
		MaterialBase::UpdateSceneData();
		SetValue("Light", "viewPos", Control::Scene::getInstance()->camera_ptr->GetViewPos());
		SetValue("Light", "direLightDir", Control::Scene::getInstance()->directionLight.GetDirection());
		SetValue("Light", "direLightColor", Control::Scene::getInstance()->directionLight.GetLightColor());
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

	void PBRMaterial::AddTexture(PBRTEXTURE_TYPE type, string name)
	{
	}

	void PBRMaterial::initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo)
	{
		cout << "PBRMaterial::initPipelineCreateInfo" << endl;
		using namespace Graphics;
		pinfo.renderPass = nameToRenderPass[RenderPassType::Default]->renderPass;
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::PBR, VK_SHADER_STAGE_VERTEX_BIT, std::move(vert_defs)));
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::PBR, VK_SHADER_STAGE_FRAGMENT_BIT, std::move(vert_defs)));

	}

	void PBRMaterial::InitPBRData()
	{
		using namespace Graphics;
		Dcb::RawLayout pbrLayout;
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
			std::transform(name.begin(), name.end(), name.begin(), std::toupper);
			frag_defs.emplace_back("BINDING_" + name + "=" + std::to_string(pbrbinding));
			addTexture(LayoutType::PBRTEXTURE, StageFlag::FRAGMENT, Draw::textureManager->nameToTex[texName].textureImageView, Draw::textureManager->nameToTex[texName].textureSampler,
				pbrbinding++);
		}
	}

}