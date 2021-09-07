#include "Drawable/PrefilterMaterial.h"

namespace Draw
{
	PrefilterMaterial::PrefilterMaterial()
	{
		using namespace Graphics;

		desc_ptr = std::make_shared<DescriptorSetCore>();

		Dcb::RawLayout pushLayout;
		pushLayout.Add<Dcb::Matrix>("mvp");
		pushLayout.Add<Dcb::Float>("roughness");
		pushLayout.Add<Dcb::UINT>("numSamples");
		pushBlock = new Dcb::Buffer(std::move(pushLayout));
		(*pushBlock)["numSamples"] = 32u;
		desc_ptr->Add(StageFlag::ALL, static_cast<uint32_t>(pushBlock->GetSizeInBytes()));


		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex["skybox_texture"].textureImageView,
			textureManager->nameToTex["skybox_texture"].textureSampler);
	}
	void PrefilterMaterial::Compile()
	{
		desc_ptr->Compile();

		loadVertexInfo();

		loadShader(Bind::ShaderType::PREFILTER);
		
		VkGraphicsPipelineCreateInfo pipelineCI = Graphics::initializers::pipelineCreateInfo(desc_ptr->GetPipelineLayout(), Graphics::nameToRenderPass[Graphics::RenderPassType::PREFILTER]->renderPass, 0);
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		depthStencilState.depthTestEnable = VK_FALSE;
		depthStencilState.depthWriteEnable = VK_FALSE;
		pipelineCI.pInputAssemblyState = &inputAssemblyState;
		pipelineCI.pRasterizationState = &rasterizationState;
		pipelineCI.pColorBlendState = &colorBlendState;
		pipelineCI.pMultisampleState = &multisampleState;
		pipelineCI.pDepthStencilState = &depthStencilState;
		pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCI.pStages = shaderStages.data();
		pipelineCI.pVertexInputState = &vertexInputInfo;
		pipelineCI.pDynamicState = &dynamicState;
		pipelineCI.pViewportState = &viewportState;

		if (vkCreateGraphicsPipelines(Graphics::Vulkan::getInstance()->GetDevice().device, NULL, 1, &pipelineCI, NULL, &pipeline)) {
			throw std::runtime_error("Failed to create a graphics pipeline for the geometry pass.\n");
		}
	}
	void PrefilterMaterial::Execute(shared_ptr<Graphics::CommandBuffer> cmdBuf)
	{
		cout << "PrefilterMaterial::Execute" << endl;
		auto layoutCmd = cmdBuf->beginSingleTimeCommands();
		Graphics::Image::getInstance()->setImageLayout(
			layoutCmd,
			Draw::textureManager->nameToTex[framebuffer_attachment].textureImage,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		cmdBuf->endSingleTimeCommands(layoutCmd);

		auto cmd = cmdBuf->beginSingleTimeCommands();

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = numMips;
		subresourceRange.layerCount = 6;

		Graphics::Image::getInstance()->setImageLayout(
			cmd,
			Draw::textureManager->nameToTex[resources].textureImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange);

		VkViewport viewport = Graphics::initializers::viewportOffscreen((float)dim, (float)dim, 0.0f, 1.0f);
		VkRect2D scissor = Graphics::initializers::rect2D(dim, dim, 0, 0);

		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdSetScissor(cmd, 0, 1, &scissor);

		// Render

		VkClearValue clearValues[1];
		clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };

		VkRenderPassBeginInfo renderPassBeginInfo = Graphics::initializers::renderPassBeginInfo();
		// Reuse render pass from example pass
		renderPassBeginInfo.renderPass = Graphics::nameToRenderPass[Graphics::RenderPassType::PREFILTER]->renderPass;
		renderPassBeginInfo.framebuffer = Graphics::nameToRenderPass[Graphics::RenderPassType::PREFILTER]->framebuffer;
		renderPassBeginInfo.renderArea.extent.width = dim;
		renderPassBeginInfo.renderArea.extent.height = dim;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = clearValues;

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


		for (uint32_t m = 0; m < numMips; m++) {
			(*pushBlock)["roughness"]= (float)m / (float)(numMips - 1);
			for (uint32_t f = 0; f < 6; f++) {
				viewport = Graphics::initializers::viewportOffscreen(static_cast<float>(dim * std::pow(0.5f, m)), static_cast<float>(dim * std::pow(0.5f, m)), 0.0, 1.0);

				vkCmdSetViewport(cmd, 0, 1, &viewport);
				// Render scene from cube face's point of view
				vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
				// Update shader push constant block
				(*pushBlock)["mvp"]= glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];
				vkCmdPushConstants(cmd, desc_ptr->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushBlock->GetSizeInBytes(), pushBlock->GetData());

				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
				VkBuffer vertexBuffers[] = { vBuffer_ptr->Get() };
				auto indexBuffer = iBuffer_ptr->Get();
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
				vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, desc_ptr->GetPipelineLayout(), 0,
					static_cast<uint32_t>(desc_ptr->descriptorSets[0].size()), desc_ptr->descriptorSets[0].data(), 0, nullptr);
				vkCmdDrawIndexed(cmd, static_cast<uint32_t>(iBuffer_ptr->GetCount()), 1, 0, 0, 0);
				vkCmdEndRenderPass(cmd);
				
				Graphics::Image::getInstance()->CopyFrameBufferToImage(cmd, Draw::textureManager->nameToTex[framebuffer_attachment].textureImage, Draw::textureManager->nameToTex[resources].textureImage, f, dim, m, viewport.width, viewport.height);
			}
		}

		Graphics::Image::getInstance()->setImageLayout(
			cmd,
			Draw::textureManager->nameToTex[resources].textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			subresourceRange);
		cmdBuf->endSingleTimeCommands(cmd);
		cout << "PrefilterMaterial::Execute End" << endl;
	}
	void PrefilterMaterial::BindMeshData(shared_ptr<Bind::VertexBuffer> vBuffer_ptr, shared_ptr<Bind::IndexBuffer> iBuffer_ptr)
	{
		this->vBuffer_ptr = vBuffer_ptr;
		this->iBuffer_ptr = iBuffer_ptr;
	}
}