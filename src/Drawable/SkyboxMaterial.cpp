#include "Drawable/SkyboxMaterial.h"

namespace Draw
{
	SkyboxMaterial::SkyboxMaterial() : MaterialBase()
	{
		using namespace Graphics;
		desc_ptr->Add(LayoutType::SCENE, DescriptorType::TEXTURE2D, StageFlag::FRAGMENT);

		desc_layout_ptr = make_unique<DescriptorSetLayout>();

		desc_layout_ptr->Add(LayoutType::SCENE, DescriptorType::UNIFORM, StageFlag::VERTEX);
		desc_layout_ptr->Add(LayoutType::SCENE, DescriptorType::UNIFORM, StageFlag::FRAGMENT);
		desc_layout_ptr->Add(LayoutType::MODEL, DescriptorType::UNIFORM, StageFlag::VERTEX);

		desc_layout_ptr->Add(LayoutType::SCENE, DescriptorType::TEXTURE2D, StageFlag::FRAGMENT);
		desc_layout_ptr->Compile();

		vShader_ptr = make_unique<Bind::VertexShader>("../src/shaders/skybox.vert.glsl", "../src/shaders", "main");
		pShader_ptr = make_unique<Bind::PixelShader>("../src/shaders/skybox.frag.glsl", "../src/shaders", "main");
		AddCubeTexture("skybox_texture");
	}
	void SkyboxMaterial::AddCubeTexture(string skybox_texture_name)
	{
		using namespace Graphics;
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, textureManager->nameToTex[skybox_texture_name].textureImageView,
			textureManager->nameToTex[skybox_texture_name].textureSampler);
	}
	void SkyboxMaterial::Compile()
	{
		desc_ptr->Compile(desc_layout_ptr);
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = Graphics::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationState = Graphics::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
		VkPipelineColorBlendAttachmentState blendAttachmentState = Graphics::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
		VkPipelineColorBlendStateCreateInfo colorBlendState = Graphics::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
		VkPipelineDepthStencilStateCreateInfo depthStencilState = Graphics::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewportState = Graphics::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
		VkPipelineMultisampleStateCreateInfo multisampleState = Graphics::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
		
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vBuffer_ptr->attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &vBuffer_ptr->bindingDescription;;
		vertexInputInfo.pVertexAttributeDescriptions = vBuffer_ptr->attributeDescriptions.data();

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

		VkPipelineShaderStageCreateInfo vertex_shader_stage = {};
		vertex_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertex_shader_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertex_shader_stage.module = vShader_ptr->shader.module;
		vertex_shader_stage.pName = "main";

		VkPipelineShaderStageCreateInfo frag_shader_stage = {};
		frag_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage.module = pShader_ptr->shader.module;
		frag_shader_stage.pName = "main";

		shaderStages.emplace_back(std::move(vertex_shader_stage));
		shaderStages.emplace_back(std::move(frag_shader_stage));

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

		VkGraphicsPipelineCreateInfo pipelineCI = Graphics::initializers::pipelineCreateInfo(desc_layout_ptr->pipelineLayout, Graphics::nameToRenderPass[Graphics::RenderPassType::Default]->renderPass, 0);
		pipelineCI.pInputAssemblyState = &inputAssemblyState;
		pipelineCI.pRasterizationState = &rasterizationState;
		pipelineCI.pColorBlendState = &colorBlendState;
		pipelineCI.pMultisampleState = &multisampleState;
		pipelineCI.pViewportState = &viewportState;
		pipelineCI.pDepthStencilState = &depthStencilState;
		pipelineCI.stageCount = shaderStages.size();
		pipelineCI.pStages = shaderStages.data();
		pipelineCI.pVertexInputState = &vertexInputInfo;
		pipelineCI.pViewportState = &viewport_info;

		// Skybox pipeline (background cube)
		rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		vkCreateGraphicsPipelines(Graphics::Vulkan::getInstance()->GetDevice().device, NULL, 1, &pipelineCI, nullptr, &pipeline);
	}
}