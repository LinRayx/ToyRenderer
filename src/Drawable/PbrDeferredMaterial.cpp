#include "Drawable/PbrDeferredMaterial.h"
#include "Drawable/Shadow/CascadeShadowMaterial.h"
 //#define PRINT_CSM
namespace Draw
{
	/// <summary>
	// layout(set = 0, binding = 2) uniform sampler2D brdfLUT;
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
		using namespace Graphics;
		Dcb::RawLayout layout2;
		layout2.Add<Dcb::Float3>("viewPos");
		layout2.Add<Dcb::Matrix>("viewMat");
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

		Dcb::RawLayout directionLight;
		directionLight.Add<Dcb::Float4>("cascadeSplits");
		directionLight.Add<Dcb::Array>("cascadeViewProjMat");
		directionLight["cascadeViewProjMat"].Set<Dcb::Matrix>(SHADOWMAP_COUNT);
		directionLight.Add<Dcb::Matrix>("inverseViewMat");
		directionLight.Add<Dcb::Float3>("lightDir");
#ifdef PRINT_CSM
		directionLight.Add<Dcb::Float>("CASCADEINDEX");
#endif // PRINT_CSM

		addLayout("DirectionLight", std::move(directionLight), LayoutType::Light, DescriptorType::UNIFORM, StageFlag::FRAGMENT);
		addTexture(LayoutType::Light, StageFlag::FRAGMENT, textureManager->nameToTex["casDepth"].textureImageView, textureManager->nameToTex["casDepth"].textureSampler, DescriptorType::TEXTURE_DEPTH);

	}

	void PbrDeferredMaterial::UpdateSceneData()
	{
		SetValue("Light", "viewPos", Control::Scene::getInstance()->camera_ptr->GetViewPos());
		SetValue("Light", "viewMat", Control::Scene::getInstance()->camera_ptr->GetViewMatrix());
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

		SetValue("DirectionLight", "cascadeSplits", Cascades::GetCascadeSplits());
		{
			// auto = glm::mat4*
			auto tmp = Cascades::GetCascadeViewProjMat();
			
			for (int i = 0; i < SHADOWMAP_COUNT; ++i) {
				glm::mat4 mat = tmp[i];
				SetValue("DirectionLight", "cascadeViewProjMat", i, tmp[i]);
			}
		}
		SetValue("DirectionLight", "inverseViewMat", Cascades::GetInverseViewMat());
		SetValue("DirectionLight", "lightDir", Control::Scene::getInstance()->directionLight.GetDirection());
#ifdef PRINT_CSM
		SetValue("DirectionLight", "CASCADEINDEX", static_cast<float>(Gloable::CASCADEINDEX));
#endif
	}
	void PbrDeferredMaterial::BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd)
	{
		auto& drawCmdBuffers = cmd->drawCmdBuffers;
		auto& rp = Graphics::nameToRenderPass[Graphics::RenderPassType::LIGHT];
		VkRenderPassBeginInfo renderPassBeginInfo = Graphics::initializers::renderPassBeginInfo();
		renderPassBeginInfo.framebuffer = rp->framebuffer;
		renderPassBeginInfo.renderPass = rp->renderPass;
		renderPassBeginInfo.renderArea.extent.width = rp->width;
		renderPassBeginInfo.renderArea.extent.height = rp->height;
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(rp->clearValues.size());
		renderPassBeginInfo.pClearValues = rp->clearValues.data();

		for (size_t i = 0; i < drawCmdBuffers.size(); i++) {
			vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, desc_ptr->GetPipelineLayout(), 0,
				static_cast<uint32_t>(desc_ptr->descriptorSets[i].size()), desc_ptr->descriptorSets[i].data(), 0, nullptr);
			vkCmdDraw(drawCmdBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(drawCmdBuffers[i]);
		}
	}
	void PbrDeferredMaterial::initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo)
	{
		cout << "PbrDeferredMaterial::initPipelineCreateInfo" << endl;
		using namespace Graphics;
		frag_defs.emplace_back("SHADOW_MAP_CASCADE_COUNT=" + to_string(SHADOWMAP_COUNT));
// #define DEBUG_CSM
#ifdef DEBUG_CSM
		frag_defs.emplace_back("DEBUG_CSM");
#endif // DEBUG_CSM


#ifdef PRINT_CSM
		frag_defs.emplace_back("PRINT_CSM");
#endif // PRINT_CSM


		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::FULLSCREEN_VERT, VK_SHADER_STAGE_VERTEX_BIT, std::move(vert_defs)));
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::PBR_Deferred, VK_SHADER_STAGE_FRAGMENT_BIT, std::move(frag_defs)));
		pinfo.pVertexInputState = &emptyVertexInputState;
		pinfo.renderPass = nameToRenderPass[RenderPassType::LIGHT]->renderPass;
		colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
		colorBlendState.pAttachments = blendAttachmentStates.data();
		depthStencilState.depthTestEnable = VK_FALSE;
		depthStencilState.depthWriteEnable = VK_FALSE;
	}
}