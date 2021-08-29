#include "Drawable/PhoneMaterial.h"

namespace Draw
{
	PhoneMaterial::PhoneMaterial() : MaterialBase()
	{
		using namespace Graphics;


		desc_layout_ptr->Add(LayoutType::SCENE, DescriptorType::UNIFORM, StageFlag::VERTEX);
		desc_layout_ptr->Add(LayoutType::SCENE, DescriptorType::UNIFORM, StageFlag::FRAGMENT);
		desc_layout_ptr->Add(LayoutType::MODEL, DescriptorType::UNIFORM, StageFlag::VERTEX);
		desc_layout_ptr->Add(LayoutType::MODEL, DescriptorType::UNIFORM, StageFlag::FRAGMENT);
		desc_layout_ptr->Add(LayoutType::MODEL, DescriptorType::TEXTURE2D, StageFlag::FRAGMENT);
		desc_layout_ptr->Add(LayoutType::MODEL, DescriptorType::TEXTURE2D, StageFlag::FRAGMENT);
		desc_layout_ptr->Compile();

		Dcb::RawLayout textureFlags;
		textureFlags.Add<Dcb::Bool>("HasDiffuseTex");
		textureFlags.Add<Dcb::Bool>("HasSpecularTex");
		addLayout("TextureFlags", std::move(textureFlags), LayoutType::MODEL, DescriptorType::UNIFORM, StageFlag::FRAGMENT);
		matType = MaterialType::Phone;
	}
	void PhoneMaterial::LoadModelTexture(const aiMaterial* material, string directory, string meshName)
	{
		loadTextures(material, aiTextureType_DIFFUSE, directory, meshName);
		loadTextures(material, aiTextureType_SPECULAR, directory, meshName);
	}
	void PhoneMaterial::Compile()
	{
		desc_ptr->Compile(desc_layout_ptr);
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vBuffer_ptr->attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &vBuffer_ptr->bindingDescription;;
		vertexInputInfo.pVertexAttributeDescriptions = vBuffer_ptr->attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		vector<VkPipelineShaderStageCreateInfo> stages;

		VkPipelineShaderStageCreateInfo vertex_shader_stage = {};
		vertex_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertex_shader_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertex_shader_stage.module = Bind::shaderFactory[Bind::ShaderType::Phone]->vert_shader->GetShaderModule();
		vertex_shader_stage.pName = "main";

		VkPipelineShaderStageCreateInfo frag_shader_stage = {};
		frag_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage.module = Bind::shaderFactory[Bind::ShaderType::Phone]->frag_shader->GetShaderModule();
		frag_shader_stage.pName = "main";

		stages.emplace_back(std::move(vertex_shader_stage));
		stages.emplace_back(std::move(frag_shader_stage));

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)Graphics::Vulkan::getInstance()->GetSwapchain().extent.width;
		viewport.height = (float)Graphics::Vulkan::getInstance()->GetSwapchain().extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;


		VkRect2D scissor = { .extent = Graphics::Vulkan::getInstance()->GetSwapchain().extent };

		VkPipelineViewportStateCreateInfo viewport_info = {};
		viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_info.viewportCount = 1;
		viewport_info.scissorCount = 1;
		viewport_info.pScissors = &scissor;
		viewport_info.pViewports = &viewport;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(1);
		for (size_t i = 0; i < colorBlendAttachments.size(); ++i) {
			colorBlendAttachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachments[i].blendEnable = VK_FALSE;
		}


		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
		colorBlending.pAttachments = colorBlendAttachments.data();
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = stages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewport_info;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		auto depthStencilState = Bind::depthStencilState_ptr->GetDepthStencilState(Bind::DepthStencilStateType::WriteStencil);
		pipelineInfo.pDepthStencilState = &depthStencilState;

		pipelineInfo.layout = desc_layout_ptr->pipelineLayout;
		pipelineInfo.renderPass = Graphics::nameToRenderPass[Graphics::RenderPassType::Default]->renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(Graphics::Vulkan::getInstance()->GetDevice().device, NULL, 1, &pipelineInfo, NULL, &pipeline)) {
			throw std::runtime_error("Failed to create a graphics pipeline for the geometry pass.\n");
		}
	}
}