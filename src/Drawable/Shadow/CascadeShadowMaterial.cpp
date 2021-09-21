#include "Drawable/Shadow/CascadeShadowMaterial.h"
#include <glm/gtx/string_cast.hpp>
namespace Draw
{
	namespace Cascades
	{
		struct Cascade
		{
			float splitDepth;
			glm::mat4 viewProjMatrix;
		} cascades[SHADOWMAP_COUNT];

		float cascadeSplits[SHADOWMAP_COUNT];
		glm::mat4 cascadeViewProjMat[SHADOWMAP_COUNT];
		glm::mat4 inverseViewMat;
		uint32_t cascadeIndex;
		float cascadeSplitLambda = 0.85f;

		float& GetCascadeSplitLambda()
		{
			return cascadeSplitLambda;
		}

		void updateCascade()
		{
			// std::cout << "updateCascade" << std::endl;
			float cascadeSplits[SHADOWMAP_COUNT];
			auto& camera = Control::Scene::getInstance()->camera_ptr;
			float nearClip = camera->GetNearPlane();
			float farClip = camera->GetFarPlane();
			float clipRange = farClip - nearClip;

			float minZ = nearClip;
			float maxZ = nearClip + clipRange;

			float range = maxZ - minZ;
			float ratio = maxZ / minZ;

			// Calculate split depths based on view camera frustum
			// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
			for (uint32_t i = 0; i < SHADOWMAP_COUNT; i++) {
				float p = (i + 1) / static_cast<float>(SHADOWMAP_COUNT);
				float log = minZ * std::pow(ratio, p);
				float uniform = minZ + range * p;
				float d = cascadeSplitLambda * (log - uniform) + uniform;
				cascadeSplits[i] = (d - nearClip) / clipRange;
			}

			// Calculate orthographic projection matrix for each cascade
			float lastSplitDist = 0.0;
			for (uint32_t i = 0; i < SHADOWMAP_COUNT; i++) {
				float splitDist = cascadeSplits[i];

				glm::vec3 frustumCorners[8] = {
					glm::vec3(-1.0f,  1.0f, -1.0f),
					glm::vec3(1.0f,  1.0f, -1.0f),
					glm::vec3(1.0f, -1.0f, -1.0f),
					glm::vec3(-1.0f, -1.0f, -1.0f),
					glm::vec3(-1.0f,  1.0f,  1.0f),
					glm::vec3(1.0f,  1.0f,  1.0f),
					glm::vec3(1.0f, -1.0f,  1.0f),
					glm::vec3(-1.0f, -1.0f,  1.0f),
				};

				// Project frustum corners into world space
				glm::mat4 invCam = glm::inverse(camera->GetProjectMatrix() * camera->GetViewMatrix());
				for (uint32_t i = 0; i < 8; i++) {
					glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
					frustumCorners[i] = invCorner / invCorner.w;
				}

				for (uint32_t i = 0; i < 4; i++) {
					glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
					frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
					frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
				}

				// Get frustum center
				glm::vec3 frustumCenter = glm::vec3(0.0f);
				for (uint32_t i = 0; i < 8; i++) {
					frustumCenter += frustumCorners[i];
				}
				frustumCenter /= 8.0f;

				float radius = 0.0f;
				for (uint32_t i = 0; i < 8; i++) {
					float distance = glm::length(frustumCorners[i] - frustumCenter);
					radius = glm::max(radius, distance);
				}
				radius = std::ceil(radius * 16.0f) / 16.0f;

				glm::vec3 maxExtents = glm::vec3(radius);
				glm::vec3 minExtents = -maxExtents;

				glm::vec3 lightDir = normalize(Control::Scene::getInstance()->directionLight.GetDirection());
				glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * maxExtents.z, frustumCenter, glm::vec3(0, 1, 0));
				glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z * 2);
				// std::cout << glm::to_string(frustumCenter) << " " << glm::to_string(frustumCenter - lightDir * maxExtents.z) << std::endl;
				// Store split distance and matrix in cascade

				cascades[i].splitDepth = (camera->GetNearPlane() + splitDist * clipRange) * -1.0f;
				cascades[i].viewProjMatrix = lightOrthoMatrix * lightViewMatrix;
				cascadeViewProjMat[i] = lightOrthoMatrix * lightViewMatrix;
				// cout << glm::to_string(lightOrthoMatrix) <<" " << glm::to_string(lightViewMatrix)<< endl;
				lastSplitDist = cascadeSplits[i];

				//// ShadowMap Texel Align
				//glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
				//shadowOrigin = cascadeViewProjMat[i] * shadowOrigin;
				//shadowOrigin = shadowOrigin * ((float)Gloable::CSM_MAP_DIM * 0.5f);
				//glm::vec4 roundOrign = glm::round(shadowOrigin);
				//glm::vec4 roundOffset = roundOrign - shadowOrigin;
				//roundOffset = roundOffset * 2.0f / (float)Gloable::CSM_MAP_DIM;
				//lightOrthoMatrix[3][0] = lightOrthoMatrix[3][0] + roundOffset.x;
				//lightOrthoMatrix[3][1] = lightOrthoMatrix[3][1] + roundOffset.y;
				//cascadeViewProjMat[i] = lightOrthoMatrix * lightViewMatrix;

			}
		}
		glm::vec4 GetCascadeSplits()
		{
			return glm::vec4(cascades[0].splitDepth, cascades[1].splitDepth,
				cascades[2].splitDepth, cascades[3].splitDepth);
		}
		glm::mat4* GetCascadeViewProjMat()
		{
			return cascadeViewProjMat;
		}
		glm::mat4 GetInverseViewMat()
		{
			return inverseViewMat;
		}
	}

	CascadeShadowMaterial::CascadeShadowMaterial() : MaterialBase()
	{
		Dcb::RawLayout transBuf;
		transBuf.Add<Dcb::Matrix>("modelTrans");
		addLayout("Model", std::move(transBuf), Graphics::LayoutType::MODEL, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::VERTEX);

		Dcb::RawLayout layout;
		layout.Add<Dcb::Array>("csmViewProjs");
		layout["csmViewProjs"].Set<Dcb::Matrix>(SHADOWMAP_COUNT);
		addLayout("Cascades", std::move(layout), Graphics::LayoutType::SCENE, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::VERTEX);
	
		// push constant
		desc_ptr->Add(Graphics::StageFlag::VERTEX, sizeof(uint32_t));
		SetDirectionLight(Control::Scene::getInstance()->directionLight);
	}

	void CascadeShadowMaterial::BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd, int index, int face)
	{
		auto& drawCmdBuffer = cmd->drawCmdBuffers[index];
		VkBuffer vertexBuffers[] = { vBuffer_ptr->Get() };
		auto indexBuffer = iBuffer_ptr->Get();
		VkDeviceSize offsets[] = { 0 };

		vkCmdPushConstants(drawCmdBuffer, desc_ptr->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &face);
		vkCmdBindPipeline(drawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdBindDescriptorSets(drawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, desc_ptr->GetPipelineLayout(), 0,
			static_cast<uint32_t>(desc_ptr->descriptorSets[index].size()), desc_ptr->descriptorSets[index].data(), 0, nullptr);
		vkCmdBindVertexBuffers(drawCmdBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(drawCmdBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(drawCmdBuffer, static_cast<uint32_t>(iBuffer_ptr->GetCount()), 1, 0, 0, 0);
	}


	void CascadeShadowMaterial::initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo)
	{
		cout << "CascadeShadowMaterial::initPipelineCreateInfo" << endl;

		using namespace Graphics;
		vert_defs.emplace_back("SHADOW_MAP_CASCADE_COUNT=" + std::to_string(SHADOWMAP_COUNT));
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::CASCADE_SHADOW, VK_SHADER_STAGE_VERTEX_BIT, std::move(vert_defs)));
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::CASCADE_SHADOW, VK_SHADER_STAGE_FRAGMENT_BIT, std::move(frag_defs)));
		pinfo.renderPass = nameToRenderPass[RenderPassType::CASCADE_SHADOW]->renderPass;
		viewport_info.pScissors = &scissor;
		viewport_info.pViewports = &viewport;
		colorBlendState.attachmentCount = 0;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		// Enable depth clamp (if available)
		rasterizationState.depthClampEnable = VK_TRUE;
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
	}
	void CascadeShadowMaterial::UpdateSceneData()
	{
		Cascades::updateCascade();
		for (int i = 0; i < SHADOWMAP_COUNT; ++i) {
			SetValue("Cascades", "csmViewProjs", i, Cascades::cascades[i].viewProjMatrix);
		}
	}
	void CascadeShadowMaterial::SetDirectionLight(Control::DirectionLight& dl)
	{
		light_ptr = &dl;
	}


}