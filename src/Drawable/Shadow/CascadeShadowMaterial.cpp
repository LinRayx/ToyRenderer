#include "Drawable/Shadow/CascadeShadowMaterial.h"

namespace Draw
{
	CascadeShadowMaterial::CascadeShadowMaterial() : MaterialBase()
	{
		Dcb::RawLayout layout;
		layout.Add<Dcb::Array>("csmViewProjs");
		layout["csmViewProjs"].Set<Dcb::Matrix>(SHADOWMAP_COUNT);
		addLayout("Cascades", std::move(layout), Graphics::LayoutType::SCENE, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::VERTEX);
	}

	void CascadeShadowMaterial::BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd)
	{
		auto& drawCmdBuffers = cmd->drawCmdBuffers;
		VkClearValue clearValues[1];
		clearValues[0].depthStencil = { 1.0f, 0 };

		auto& rp = Graphics::nameToRenderPass[Graphics::RenderPassType::CASCADE_SHADOW];
		VkRenderPassBeginInfo renderPassBeginInfo = Graphics::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = rp->renderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = SHADOWMAP_DIM;
		renderPassBeginInfo.renderArea.extent.height = SHADOWMAP_DIM;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = clearValues;

		for (size_t i = 0; i < drawCmdBuffers.size(); ++i) {
			// One pass per cascade
			// The layer that this pass renders to is defined by the cascade's image view (selected via the cascade's descriptor set)

			VkDeviceSize offsets[] = { 0 };
			for (uint32_t j = 0; j < SHADOWMAP_COUNT; j++) {
				renderPassBeginInfo.framebuffer = rp->framebuffers[j];
				vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
				vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
				vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, desc_ptr->GetPipelineLayout(), 0,
					static_cast<uint32_t>(desc_ptr->descriptorSets[i].size()), desc_ptr->descriptorSets[i].data(), 0, nullptr);
				for (size_t k = 0; k < vBuffer_ptrs.size(); ++k) {
					VkBuffer vertexBuffers[] = { vBuffer_ptrs[k]->Get() };
					auto indexBuffer = iBuffer_ptrs[k]->Get();
					vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, vertexBuffers, offsets);
					vkCmdBindIndexBuffer(drawCmdBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
					vkCmdDrawIndexed(drawCmdBuffers[i], static_cast<uint32_t>(iBuffer_ptrs[k]->GetCount()), 1, 0, 0, 0);
				}
				vkCmdEndRenderPass(drawCmdBuffers[i]);
			}
		}
	}

	void CascadeShadowMaterial::CollectVAndIBuffers(shared_ptr<Bind::VertexBuffer> v_ptr, shared_ptr<Bind::IndexBuffer> i_ptr)
	{
		vBuffer_ptrs.emplace_back(v_ptr);
		iBuffer_ptrs.emplace_back(i_ptr);
	}

	void CascadeShadowMaterial::initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo)
	{
		cout << "CascadeShadowMaterial::initPipelineCreateInfo" << endl;

		using namespace Graphics;
		frag_defs.emplace_back("SHADOW_MAP_CASCADE_COUNT=" + std::to_string(SHADOWMAP_COUNT));
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::CASCADE_SHADOW, VK_SHADER_STAGE_VERTEX_BIT, std::move(vert_defs)));
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::CASCADE_SHADOW, VK_SHADER_STAGE_FRAGMENT_BIT, std::move(frag_defs)));
		pinfo.renderPass = nameToRenderPass[RenderPassType::CASCADE_SHADOW]->renderPass;
		viewport_info.pScissors = &scissor;
		viewport_info.pViewports = &viewport;
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
	}
	void CascadeShadowMaterial::UpdateSceneData()
	{
		updateCascade();
		for (int i = 0; i < SHADOWMAP_COUNT; ++i) {
			SetValue("Cascades", "csmViewProjs", i, cascades[i].viewProjMatrix);
		}
	}
	void CascadeShadowMaterial::SetDirectionLight(Control::DirectionLight& dl)
	{
		light_ptr = &dl;
	}
	void CascadeShadowMaterial::updateCascade()
	{
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

			glm::vec3 lightDir = normalize(light_ptr->GetDirection());
			glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

			// Store split distance and matrix in cascade
			cascades[i].splitDepth = (camera->GetNearPlane() + splitDist * clipRange) * -1.0f;
			cascades[i].viewProjMatrix = lightOrthoMatrix * lightViewMatrix;

			lastSplitDist = cascadeSplits[i];
		}

	}
}