#include "Drawable/Shadow/OnmiShadowGenMaterial.h"
#include <glm/gtx/string_cast.hpp>
namespace Draw
{
	OnmiShadowGenMaterial::OnmiShadowGenMaterial() : MaterialBase()
	{
		using namespace Graphics;

		Dcb::RawLayout layout;
		layout.Add<Dcb::Matrix>("projMat");
		layout.Add<Dcb::Float3>("lightPos");
		addLayout("Scene", std::move(layout), LayoutType::SCENE, DescriptorType::UNIFORM, StageFlag::VERTEX);
		SetValue("Scene", "projMat", shadowProj);

		Dcb::RawLayout transBuf;
		transBuf.Add<Dcb::Matrix>("modelTrans");
		addLayout("Model", std::move(transBuf), LayoutType::MODEL, DescriptorType::UNIFORM, StageFlag::VERTEX);

		Dcb::RawLayout pushLayout;
		pushLayout.Add<Dcb::Matrix>("view");
		pushBlock = new Dcb::Buffer(std::move(pushLayout));
		desc_ptr->Add(StageFlag::VERTEX, static_cast<uint32_t>(pushBlock->GetSizeInBytes()));
	}

	void OnmiShadowGenMaterial::BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd, int index, int face)
	{
		glm::mat4 viewMatrix = glm::mat4(1.0f);
		switch (face)
		{
		case 0: // POSITIVE_X
			viewMatrix = glm::rotate(viewMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			break;
		case 1:	// NEGATIVE_X
			viewMatrix = glm::rotate(viewMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			break;
		case 2:	// POSITIVE_Y
			viewMatrix = glm::rotate(viewMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			break;
		case 3:	// NEGATIVE_Y
			viewMatrix = glm::rotate(viewMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			break;
		case 4:	// POSITIVE_Z
			viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			break;
		case 5:	// NEGATIVE_Z
			viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			break;
		}
		
		auto& drawCmdBuffer = cmd->drawCmdBuffers[index];
			(*pushBlock)["view"] = viewMatrix;
			vkCmdPushConstants(drawCmdBuffer, desc_ptr->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, static_cast<uint32_t>(pushBlock->GetSizeInBytes()), pushBlock->GetData());
			vkCmdBindPipeline(drawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			VkBuffer vertexBuffers[] = { vBuffer_ptr->Get() };
			auto indexBuffer = iBuffer_ptr->Get();
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(drawCmdBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(drawCmdBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdBindDescriptorSets(drawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, desc_ptr->GetPipelineLayout(), 0,
				static_cast<uint32_t>(desc_ptr->descriptorSets[index].size()), desc_ptr->descriptorSets[index].data(), 0, nullptr);
			vkCmdDrawIndexed(drawCmdBuffer, static_cast<uint32_t>(iBuffer_ptr->GetCount()), 1, 0, 0, 0);
		
	}

	void OnmiShadowGenMaterial::UpdateSceneData()
	{
		SetValue("Scene", "lightPos", pl_ptr->GetLightPosition());
	}
	void OnmiShadowGenMaterial::SetTransform(glm::mat4 translate, glm::mat4 rotate)
	{
		auto lightPos = pl_ptr->GetLightPosition();
		translate = glm::translate(translate, glm::vec3(-lightPos.x, -lightPos.y, -lightPos.z));
		SetValue("Model", "modelTrans",  translate * rotate);
	}
	void OnmiShadowGenMaterial::initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo)
	{
		cout << "OnmiShadowGenMaterial::initPipelineCreateInfo" << endl;
		using namespace Graphics;
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::OMNISHADOW, VK_SHADER_STAGE_VERTEX_BIT, std::move(vert_defs)));
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::OMNISHADOW, VK_SHADER_STAGE_FRAGMENT_BIT, std::move(frag_defs)));
		viewport_info.pScissors = &scissor;
		viewport_info.pViewports = &viewport;
		pinfo.renderPass = nameToRenderPass[RenderPassType::ONMISHADOW]->renderPass;
	}
}