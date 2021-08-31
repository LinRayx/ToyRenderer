#include "Drawable/PBRMaterial.h"

namespace Draw
{
	PBRMaterial::PBRMaterial()
		:MaterialBase()
	{
		using namespace Graphics;

		Dcb::RawLayout pbrLayout;
		pbrLayout.Add<Dcb::Float>("metallic");
		pbrLayout.Add<Dcb::Float>("roughness");
		pbrLayout.Add<Dcb::Bool>("HasDiffuseTex");

		addLayout("PbrParam", std::move(pbrLayout), LayoutType::MODEL, DescriptorType::UNIFORM, StageFlag::FRAGMENT);
		
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["brdf_lut"].textureImageView, Draw::textureManager->nameToTex["brdf_lut"].textureSampler);
		AddCubeTexture("irradiance_map");
		AddCubeTexture("prefilter_map");
		addTexture(LayoutType::MODEL, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["albedoMap"].textureImageView, Draw::textureManager->nameToTex["albedoMap"].textureSampler);
		addTexture(LayoutType::MODEL, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["metallicMap"].textureImageView, Draw::textureManager->nameToTex["metallicMap"].textureSampler);
		addTexture(LayoutType::MODEL, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["normalMap"].textureImageView, Draw::textureManager->nameToTex["normalMap"].textureSampler);
		addTexture(LayoutType::MODEL, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["roughnessMap"].textureImageView, Draw::textureManager->nameToTex["roughnessMap"].textureSampler);
		addTexture(LayoutType::MODEL, StageFlag::FRAGMENT, Draw::textureManager->nameToTex["aoMap"].textureImageView, Draw::textureManager->nameToTex["aoMap"].textureSampler);

		SetValue("PbrParam", "metallic", 1.f);
		SetValue("PbrParam", "roughness", 1.0f);
		matType = MaterialType::PBR;
	}

	void PBRMaterial::LoadModelTexture(const aiMaterial* material, string directory, string meshName)
	{
		
	}

	void PBRMaterial::Compile()
	{
		cout << "PBRMaterial::Compile()" << endl;
		desc_ptr->Compile();
		using namespace Graphics;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationState = initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
		VkPipelineColorBlendAttachmentState blendAttachmentState = initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
		VkPipelineColorBlendStateCreateInfo colorBlendState = initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
		VkPipelineMultisampleStateCreateInfo multisampleState = initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
		VkGraphicsPipelineCreateInfo pipelineCI = initializers::pipelineCreateInfo(desc_ptr->GetPipelineLayout(), nameToRenderPass[RenderPassType::Default]->renderPass);
		
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)Vulkan::getInstance()->GetSwapchain().extent.width;
		viewport.height = (float)Vulkan::getInstance()->GetSwapchain().extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = { .extent = Vulkan::getInstance()->GetSwapchain().extent };

		vector<VkPipelineShaderStageCreateInfo> stages;
		VkPipelineShaderStageCreateInfo vertex_shader_stage = {};
		vertex_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertex_shader_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertex_shader_stage.module = Bind::shaderFactory[Bind::ShaderType::PBR]->vert_shader->GetShaderModule();
		vertex_shader_stage.pName = "main";

		VkPipelineShaderStageCreateInfo frag_shader_stage = {};
		frag_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage.module = Bind::shaderFactory[Bind::ShaderType::PBR]->frag_shader->GetShaderModule();
		frag_shader_stage.pName = "main";

		stages.emplace_back(std::move(vertex_shader_stage));
		stages.emplace_back(std::move(frag_shader_stage));

		VkPipelineViewportStateCreateInfo viewport_info = {};
		viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_info.viewportCount = 1;
		viewport_info.scissorCount = 1;
		viewport_info.pScissors = &scissor;
		viewport_info.pViewports = &viewport;
		
		pipelineCI.pInputAssemblyState = &inputAssemblyState;
		pipelineCI.pRasterizationState = &rasterizationState;
		pipelineCI.pColorBlendState = &colorBlendState;
		pipelineCI.pMultisampleState = &multisampleState;
		pipelineCI.pViewportState = &viewport_info;
		auto depthStencilState = Bind::depthStencilState_ptr->GetDepthStencilState(Bind::DepthStencilStateType::WriteStencil);

		pipelineCI.pDepthStencilState = &depthStencilState;
		pipelineCI.stageCount = static_cast<uint32_t>(stages.size());
		pipelineCI.pStages = stages.data();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vBuffer_ptr->attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &vBuffer_ptr->bindingDescription;;
		vertexInputInfo.pVertexAttributeDescriptions = vBuffer_ptr->attributeDescriptions.data();

		pipelineCI.pVertexInputState = &vertexInputInfo;

		if (vkCreateGraphicsPipelines(Vulkan::getInstance()->GetDevice().device, NULL, 1, &pipelineCI, NULL, &pipeline)) {
			throw std::runtime_error("Failed to create a graphics pipeline for the geometry pass.\n");
		}
	}

	void PBRMaterial::AddCubeTexture(string cube_texture_name)
	{
		using namespace Graphics;
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex[cube_texture_name].textureImageView,
			textureManager->nameToTex[cube_texture_name].textureSampler);
	}

	
}