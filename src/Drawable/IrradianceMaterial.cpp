#include "Drawable/IrradianceMaterial.h"

namespace Draw
{
	IrradianceMaterial::IrradianceMaterial()
		: MaterialBaseParent()
	{
		using namespace Graphics;
		desc_ptr = std::make_shared<DescriptorSetCore>();

		Dcb::RawLayout pushLayout;
		pushLayout.Add<Dcb::Matrix>("mvp");

		pushBlock = new Dcb::Buffer(std::move(pushLayout));

		desc_ptr->Add(StageFlag::VERTEX, pushBlock->GetSizeInBytes());

		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex["skybox_texture"].textureImageView,
			textureManager->nameToTex["skybox_texture"].textureSampler);
	}
	void IrradianceMaterial::Compile()
	{
		desc_ptr->Compile();

		loadVertexInfo();
		loadShader(Bind::ShaderType::IRRADIANCE);

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = dim;
		viewport.height = dim;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;


		VkRect2D scissor = {};
		scissor.extent.height = dim;
		scissor.extent.width = dim;

		VkPipelineViewportStateCreateInfo viewport_info = {};
		viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_info.viewportCount = 1;
		viewport_info.scissorCount = 1;
		viewport_info.pScissors = &scissor;
		viewport_info.pViewports = &viewport;


		VkGraphicsPipelineCreateInfo pipelineCI = Graphics::initializers::pipelineCreateInfo(desc_ptr->GetPipelineLayout(), Graphics::nameToRenderPass[Graphics::RenderPassType::IRRADIANCE]->renderPass, 0);
		pipelineCI.pInputAssemblyState = &inputAssemblyState;
		pipelineCI.pRasterizationState = &rasterizationState;
		pipelineCI.pColorBlendState = &colorBlendState;
		pipelineCI.pMultisampleState = &multisampleState;
		pipelineCI.pDepthStencilState = &depthStencilState;
		pipelineCI.stageCount = shaderStages.size();
		pipelineCI.pStages = shaderStages.data();
		pipelineCI.pVertexInputState = &vertexInputInfo;
		pipelineCI.pViewportState = &viewport_info;

		if (vkCreateGraphicsPipelines(Graphics::Vulkan::getInstance()->GetDevice().device, NULL, 1, &pipelineCI, NULL, &pipeline)) {
			throw std::runtime_error("Failed to create a graphics pipeline for the geometry pass.\n");
		}
	}
	void IrradianceMaterial::Execute(shared_ptr<Graphics::CommandBuffer> cmd)
	{
		cout << "IrradianceMaterial::Execute" << endl;
		auto cmdbuf = cmd->beginSingleTimeCommands();
		std::vector<glm::mat4> matrices = {
			// POSITIVE_X
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_X
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// POSITIVE_Y
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_Y
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// POSITIVE_Z
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_Z
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		};

		// Render

		VkClearValue clearValues[1];
		clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };

		VkRenderPassBeginInfo renderPassBeginInfo = Graphics::initializers::renderPassBeginInfo();
		// Reuse render pass from example pass
		renderPassBeginInfo.renderPass = Graphics::nameToRenderPass[Graphics::RenderPassType::IRRADIANCE]->renderPass;
		renderPassBeginInfo.framebuffer = Graphics::nameToRenderPass[Graphics::RenderPassType::IRRADIANCE]->framebuffers[0];
		renderPassBeginInfo.renderArea.extent.width = dim;
		renderPassBeginInfo.renderArea.extent.height = dim;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = clearValues;

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 6;
		cmd->setImageLayout(
			cmdbuf,
			textureManager->nameToTex[irradiance_map_name].textureImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange);



		for (uint32_t f = 0; f < 6; f++) {
			// Render scene from cube face's point of view
			vkCmdBeginRenderPass(cmdbuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			glm::mat mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];
			(*pushBlock)["mvp"] = mvp;
			vkCmdPushConstants(cmdbuf, desc_ptr->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, pushBlock->GetSizeInBytes(), pushBlock->GetData());

			vkCmdBindPipeline(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			VkBuffer vertexBuffers[] = { vBuffer_ptr->Get() };
			auto indexBuffer = iBuffer_ptr->Get();
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(cmdbuf, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(cmdbuf, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdBindDescriptorSets(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, desc_ptr->GetPipelineLayout(), 0,
				static_cast<uint32_t>(desc_ptr->descriptorSets[0].size()), desc_ptr->descriptorSets[0].data(), 0, nullptr);
			vkCmdDrawIndexed(cmdbuf, static_cast<uint32_t>(iBuffer_ptr->GetCount()), 1, 0, 0, 0);
			vkCmdEndRenderPass(cmdbuf);

			cmd->CopyFrameBufferToImage(cmdbuf, "irradiance_attachment", irradiance_map_name, f, dim);
		}
		// Change image layout for all cubemap faces to transfer destination
		cmd->setImageLayout(
			cmdbuf,
			textureManager->nameToTex[irradiance_map_name].textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			subresourceRange);
		cmd->endSingleTimeCommands(cmdbuf);
	}
	void IrradianceMaterial::BindMeshData(shared_ptr<Bind::VertexBuffer> vBuffer_ptr, shared_ptr<Bind::IndexBuffer> iBuffer_ptr)
	{
		this->vBuffer_ptr = vBuffer_ptr;
		this->iBuffer_ptr = iBuffer_ptr;
	}
}