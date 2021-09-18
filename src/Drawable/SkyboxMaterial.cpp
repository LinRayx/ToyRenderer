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
	void SkyboxMaterial::UpdateSceneData()
	{
		MaterialBase::UpdateSceneData();
		SetValue("ViewAndProj", "viewMat", glm::mat4(glm::mat3(Control::Scene::getInstance()->camera_ptr->GetViewMatrix())));
	}
	void SkyboxMaterial::initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo)
	{
		cout << "SkyboxMaterial::initPipelineCreateInfo" << endl;
		using namespace Graphics;
		pinfo.renderPass = nameToRenderPass[RenderPassType::Default]->renderPass;
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::Skybox, VK_SHADER_STAGE_VERTEX_BIT, std::move(vert_defs)));
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::Skybox, VK_SHADER_STAGE_FRAGMENT_BIT, std::move(vert_defs)));
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
	}
}