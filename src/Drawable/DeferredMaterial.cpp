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

	void DeferredMaterial::UpdateSceneData()
	{
		SetValue("CameraParam", "nearPlane", Control::Scene::getInstance()->camera_ptr->GetNearPlane());
		SetValue("CameraParam", "farPlane", Control::Scene::getInstance()->camera_ptr->GetFarPlane());
		SetValue("ViewAndProj", "viewMat", Control::Scene::getInstance()->camera_ptr->GetViewMatrix());
		SetValue("ViewAndProj", "projMat", Control::Scene::getInstance()->camera_ptr->GetProjectMatrix());
	}

	void DeferredMaterial::initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo)
	{
		cout << "DeferredMaterial::initPipelineCreateInfo" << endl;
		using namespace Graphics;
		pinfo.renderPass = nameToRenderPass[RenderPassType::DEFERRED]->renderPass;

		colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
		colorBlendState.pAttachments = blendAttachmentStates.data();
		rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::GBUFFER, VK_SHADER_STAGE_VERTEX_BIT, std::move(vert_defs)));
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::GBUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, std::move(frag_defs)));

	}

}