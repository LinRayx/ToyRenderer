#include "Drawable/MaterialBaseParent.h"

namespace Draw
{
	MaterialBaseParent::MaterialBaseParent()
	{
		vertexInputInfo = {};
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)Graphics::Vulkan::getInstance()->GetSwapchain().extent.width;
		viewport.height = (float)Graphics::Vulkan::getInstance()->GetSwapchain().extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;


		VkRect2D scissor = { .extent = Graphics::Vulkan::getInstance()->GetSwapchain().extent };

		viewport_info = {};
		viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_info.viewportCount = 1;
		viewport_info.scissorCount = 1;
		viewport_info.pScissors = &scissor;
		viewport_info.pViewports = &viewport;
	}

	/// <summary>
	///  FullScreen Command
	/// </summary>
	/// <param name="cmd"></param>
	void MaterialBaseParent::BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd)
	{
		auto& drawCmdBuffers = cmd->drawCmdBuffers;
		for (size_t i = 0; i < drawCmdBuffers.size(); i++) {
			vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, desc_ptr->GetPipelineLayout(), 0,
				static_cast<uint32_t>(desc_ptr->descriptorSets[i].size()), desc_ptr->descriptorSets[i].data(), 0, nullptr);
			vkCmdDraw(drawCmdBuffers[i], 3, 1, 0, 0);
		}
	}
	void MaterialBaseParent::Update(int cur)
	{
		for (auto it : buffer_ptrs)
		{
			it.second->UpdateData(cur, bufs[it.first]->GetSizeInBytes(), bufs[it.first]->GetData());
		}
	}
	void MaterialBaseParent::addLayout(std::string key, Dcb::RawLayout&& layout, Graphics::LayoutType layoutType, Graphics::DescriptorType descType, Graphics::StageFlag stage)
	{
		bufs[key] = make_shared<Dcb::Buffer>(std::move(layout));
		buffer_ptrs[key] = make_shared<Graphics::Buffer>(Graphics::BufferUsage::UNIFORM, bufs[key]->GetSizeInBytes());
		desc_ptr->Add(layoutType, descType, stage, buffer_ptrs[key]);
	}

	void MaterialBaseParent::addTexture(Graphics::LayoutType layout_type, Graphics::StageFlag stage, VkImageView textureImageView, VkSampler textureSampler)
	{
		desc_ptr->Add(layout_type, Graphics::DescriptorType::TEXTURE2D, stage, textureImageView, textureSampler);
	}
	void MaterialBaseParent::loadShader(Bind::ShaderType vert, Bind::ShaderType frag, VkSpecializationInfo specializationInfo)
	{
		if (frag == Bind::ShaderType::EMPTY)
			frag = vert;

		VkPipelineShaderStageCreateInfo vertex_shader_stage = {};
		vertex_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertex_shader_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertex_shader_stage.module = Bind::shaderFactory[vert]->vert_shader->GetShaderModule();
		vertex_shader_stage.pName = "main";

		VkPipelineShaderStageCreateInfo frag_shader_stage = {};
		frag_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage.module = Bind::shaderFactory[frag]->frag_shader->GetShaderModule();
		frag_shader_stage.pName = "main";
		frag_shader_stage.pSpecializationInfo = &specializationInfo;
	}
	void MaterialBaseParent::loadVertexInfo()
	{
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vBuffer_ptr->attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &vBuffer_ptr->bindingDescription;;
		vertexInputInfo.pVertexAttributeDescriptions = vBuffer_ptr->attributeDescriptions.data();
	}
}