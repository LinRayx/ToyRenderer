#include "Drawable/DefaultMaterial.h"

namespace Draw
{
	DefaultMaterial::DefaultMaterial(glm::vec4 color)
		: MaterialBase(true), color(color)
	{
		using namespace Graphics;
		Dcb::RawLayout pushLayout;

		pushLayout.Add<Dcb::Float4>("color");

		pushBlock = new Dcb::Buffer(std::move(pushLayout));

		desc_ptr->Add(StageFlag::FRAGMENT, static_cast<uint32_t>(pushBlock->GetSizeInBytes()));
	}

	DefaultMaterial::~DefaultMaterial()
	{
		delete pushBlock;
	}

	void DefaultMaterial::BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd)
	{
		auto& drawCmdBuffers = cmd->drawCmdBuffers;
		for (size_t i = 0; i < drawCmdBuffers.size(); i++) {
			(*pushBlock)["color"] = color;
			// vkCmdSetDepthBias(drawCmdBuffers[i], 1.25f, 0.0f, 1.75f);
			vkCmdPushConstants(drawCmdBuffers[i], desc_ptr->GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, static_cast<uint32_t>(pushBlock->GetSizeInBytes()), pushBlock->GetData());
			vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			VkBuffer vertexBuffers[] = { vBuffer_ptr->Get() };
			auto indexBuffer = iBuffer_ptr->Get();
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(drawCmdBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, desc_ptr->GetPipelineLayout(), 0,
				static_cast<uint32_t>(desc_ptr->descriptorSets[i].size()), desc_ptr->descriptorSets[i].data(), 0, nullptr);
			vkCmdDrawIndexed(drawCmdBuffers[i], static_cast<uint32_t>(iBuffer_ptr->GetCount()), 1, 0, 0, 0);
		}
	}
	glm::vec3 DefaultMaterial::GetPosition()
	{
		return glm::vec3(0.f);
	}
	bool DefaultMaterial::SetUI()
	{
		bool dirty = false;
		const auto dcheck = [&dirty](bool changed) {dirty = dirty || changed; };
		ImGui::TextColored({ 0.4f,1.0f,0.6f,1.0f }, "PBRMaterial");
		ImVec4 albedoUI = ImVec4(color.x, color.y, color.z, 1.00f);
		dcheck(ImGui::ColorEdit3("albedo", (float*)&albedoUI));
		this->color = glm::vec4(albedoUI.x, albedoUI.y, albedoUI.z, 1);
		return dirty;
	}
	void DefaultMaterial::SetColor(glm::vec4 color)
	{
		this->color = color;
	}
	glm::vec4 DefaultMaterial::GetColor()
	{
		return color;
	}
	void DefaultMaterial::initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo)
	{
		cout << "DefaultMaterial::initPipelineCreateInfo" << endl;
		using namespace Graphics;
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::DEFAULT, VK_SHADER_STAGE_VERTEX_BIT, std::move(vert_defs)));
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::DEFAULT, VK_SHADER_STAGE_FRAGMENT_BIT, std::move(vert_defs)));
		pinfo.renderPass = nameToRenderPass[RenderPassType::Default]->renderPass;
	}
}