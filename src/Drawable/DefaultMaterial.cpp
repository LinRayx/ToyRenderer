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

		desc_ptr->Add(StageFlag::FRAGMENT, pushBlock->GetSizeInBytes());
	}

	DefaultMaterial::~DefaultMaterial()
	{
		delete pushBlock;
	}

	void DefaultMaterial::Compile()
	{
		cout << "DefaultMaterial::Compile()" << endl;
		desc_ptr->Compile();
		using namespace Graphics;
		VkGraphicsPipelineCreateInfo pipelineCI = initializers::pipelineCreateInfo(desc_ptr->GetPipelineLayout(), nameToRenderPass[RenderPassType::Default]->renderPass);
		//rasterizationState.depthBiasEnable = VK_TRUE;
		//rasterizationState.depthBiasConstantFactor = 1.25f;
		////rasterizationState.depthClampEnable = VK_TRUE;
		//rasterizationState.depthBiasClamp = 0.0f;
		//rasterizationState.depthBiasSlopeFactor = 1.75f;
		loadVertexInfo();
		loadShader(Bind::ShaderType::DEFAULT);
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

	void DefaultMaterial::BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd)
	{
		auto& drawCmdBuffers = cmd->drawCmdBuffers;
		for (size_t i = 0; i < drawCmdBuffers.size(); i++) {
			(*pushBlock)["color"] = color;
			// vkCmdSetDepthBias(drawCmdBuffers[i], 1.25f, 0.0f, 1.75f);
			vkCmdPushConstants(drawCmdBuffers[i], desc_ptr->GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushBlock->GetSizeInBytes(), pushBlock->GetData());
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
}