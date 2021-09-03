#include "RenderPass.h"
#include <stdexcept>
#include <array>

namespace Graphics {

	RenderPass::~RenderPass()
	{
		for (auto& framebuffer : framebuffers) {
			vkDestroyFramebuffer(Vulkan::getInstance()->GetDevice().device, framebuffer, Vulkan::getInstance()->GetDevice().allocator);
		}
		vkDestroyRenderPass(Vulkan::getInstance()->GetDevice().device, renderPass, Vulkan::getInstance()->GetDevice().allocator);
		if (framebuffer != VK_NULL_HANDLE)
			vkDestroyFramebuffer(Vulkan::getInstance()->GetDevice().device, framebuffer, Vulkan::getInstance()->GetDevice().allocator);
	}

	RenderPass::RenderPass()
	{
		width = Vulkan::getInstance()->GetWidth();
		height = Vulkan::getInstance()->GetHeight();
	}

	void RenderPass::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = Vulkan::getInstance()->swapchain.format;
		colorAttachment.samples = msaaSamples;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
		depthAttachment.samples = msaaSamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // 设置成DONT CARE 时 outline 会出现问题
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		clearValues.resize(2);
		clearValues[0].color = { 0.0, 0, 0, 1 };
		clearValues[1].depthStencil = { 1.0, 0 };

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;


		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(Vulkan::getInstance()->device.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

		framebuffers.resize(Vulkan::getInstance()->swapchain.image_count);
		for (size_t i = 0; i < framebuffers.size(); i++) {
			std::vector<VkImageView> atts;

			atts.emplace_back(Vulkan::getInstance()->swapchain.image_views[i]);
			atts.emplace_back(Draw::textureManager->nameToTex["depth"].textureImageView);

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(atts.size());
			framebufferInfo.pAttachments = atts.data();
			framebufferInfo.width = Vulkan::getInstance()->swapchain.extent.width;
			framebufferInfo.height = Vulkan::getInstance()->swapchain.extent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(Vulkan::getInstance()->device.device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void RenderPass::CreateOffScreenRenderPass(string resource_name, VkImageLayout finalLayout)
	{
		// FB, Att, RP, Pipe, etc.
		VkAttachmentDescription attDesc = {};
		// Color attachment
		attDesc.format = Draw::textureManager->nameToTex[resource_name].format;
		attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
		attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attDesc.finalLayout = finalLayout;
		VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;

		// Use subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create the actual renderpass
		VkRenderPassCreateInfo renderPassCI{};
		renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCI.attachmentCount = 1;
		renderPassCI.pAttachments = &attDesc;
		renderPassCI.subpassCount = 1;
		renderPassCI.pSubpasses = &subpassDescription;
		renderPassCI.dependencyCount = 2;
		renderPassCI.pDependencies = dependencies.data();

		vkCreateRenderPass(Vulkan::getInstance()->GetDevice().device, &renderPassCI, nullptr, &renderPass);

		VkFramebufferCreateInfo framebufferCI{};
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.renderPass = renderPass;
		framebufferCI.attachmentCount = 1;
		framebufferCI.pAttachments = &Draw::textureManager->nameToTex[resource_name].textureImageView;
		framebufferCI.width = Draw::textureManager->nameToTex[resource_name].texWidth;
		framebufferCI.height = Draw::textureManager->nameToTex[resource_name].texHeight;
		framebufferCI.layers = 1;

		vkCreateFramebuffer(Vulkan::getInstance()->GetDevice().device,  &framebufferCI, nullptr, &framebuffer);
	}

	void RenderPass::CreateDeferredRenderPass()
	{
		clearValues.resize(5);
		clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		clearValues[3].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clearValues[4].depthStencil = { 1.0f, 0 };

		std::array<VkAttachmentDescription, 5> attachmentDescs = {};

		// Init attachment properties
		for (uint32_t i = 0; i < static_cast<uint32_t>(attachmentDescs.size()); i++)
		{
			attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDescs[i].finalLayout = (i == 4) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		
		// Formats
		attachmentDescs[0].format = Draw::textureManager->nameToTex["GBuffer_position"].format;
		attachmentDescs[1].format = Draw::textureManager->nameToTex["GBuffer_normals"].format;
		attachmentDescs[2].format = Draw::textureManager->nameToTex["GBuffer_albedo"].format;
		attachmentDescs[3].format = Draw::textureManager->nameToTex["GBuffer_metallic_roughness"].format;
		attachmentDescs[4].format = Draw::textureManager->nameToTex["GBuffer_depth"].format;

		std::vector<VkAttachmentReference> colorReferences;
		colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		colorReferences.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		colorReferences.push_back({ 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 4;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pColorAttachments = colorReferences.data();
		subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
		subpass.pDepthStencilAttachment = &depthReference;

		// Use subpass dependencies for attachment layout transitions
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pAttachments = attachmentDescs.data();
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = dependencies.data();
		vkCreateRenderPass(Vulkan::getInstance()->GetDevice().device, &renderPassInfo, nullptr, &renderPass);

		std::array<VkImageView, 5> attachments;
		attachments[0] = Draw::textureManager->nameToTex["GBuffer_position"].textureImageView;
		attachments[1] = Draw::textureManager->nameToTex["GBuffer_normals"].textureImageView;
		attachments[2] = Draw::textureManager->nameToTex["GBuffer_albedo"].textureImageView;
		attachments[3] = Draw::textureManager->nameToTex["GBuffer_metallic_roughness"].textureImageView;
		attachments[4] = Draw::textureManager->nameToTex["GBuffer_depth"].textureImageView;

		VkFramebufferCreateInfo fbufCreateInfo = initializers::framebufferCreateInfo();
		fbufCreateInfo.renderPass = renderPass;
		fbufCreateInfo.pAttachments = attachments.data();
		fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		fbufCreateInfo.width = Vulkan::getInstance()->GetWidth();
		fbufCreateInfo.height = Vulkan::getInstance()->GetHeight();
		fbufCreateInfo.layers = 1;

		vkCreateFramebuffer(Vulkan::getInstance()->GetDevice().device, &fbufCreateInfo, nullptr, &framebuffer);

	}

	void RenderPass::CreateFullScreenRenderPass(string resource_name)
	{
		clearValues.resize(2);
		clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		clearValues[1].depthStencil = { 1.0f, 0 };
		// FB, Att, RP, Pipe, etc.
		VkAttachmentDescription attDesc = {};
		// Color attachment
		attDesc.format = Draw::textureManager->nameToTex[resource_name].format;
		attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
		attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;

		// Use subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create the actual renderpass
		VkRenderPassCreateInfo renderPassCI{};
		renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCI.attachmentCount = 1;
		renderPassCI.pAttachments = &attDesc;
		renderPassCI.subpassCount = 1;
		renderPassCI.pSubpasses = &subpassDescription;
		renderPassCI.dependencyCount = 2;
		renderPassCI.pDependencies = dependencies.data();

		vkCreateRenderPass(Vulkan::getInstance()->GetDevice().device, &renderPassCI, nullptr, &renderPass);


		VkFramebufferCreateInfo framebufferCI{};
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.renderPass = renderPass;
		framebufferCI.attachmentCount = 1;
		framebufferCI.pAttachments = &Draw::textureManager->nameToTex[resource_name].textureImageView;
		framebufferCI.width = Draw::textureManager->nameToTex[resource_name].texWidth;
		framebufferCI.height = Draw::textureManager->nameToTex[resource_name].texHeight;
		framebufferCI.layers = 1;

		vkCreateFramebuffer(Vulkan::getInstance()->GetDevice().device, &framebufferCI, nullptr, &framebuffer);
	}

	map<RenderPassType, RenderPass*> nameToRenderPass;

	void InitRenderPass()
	{
		Draw::textureManager->CreateDepthResource("depth");
		Draw::textureManager->CreateResource("brdf_lut", VK_FORMAT_R16G16_SFLOAT, 512, 
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		Draw::textureManager->CreateResource("irradiance_attachment", VK_FORMAT_R32G32B32A32_SFLOAT, 64, 
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		Draw::textureManager->CreateCubeResource("irradiance_map", VK_FORMAT_R32G32B32A32_SFLOAT, 64);
		// VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
		uint32_t numMips = static_cast<uint32_t>(floor(log2(512))) + 1;
		Draw::textureManager->CreateResource("prefilter_attachment", VK_FORMAT_R16G16B16A16_SFLOAT, 512,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		Draw::textureManager->CreateCubeResource("prefilter_map", VK_FORMAT_R16G16B16A16_SFLOAT, 512, numMips);

		// deferred resouces
		Draw::textureManager->CreateResource("GBuffer_position", VK_FORMAT_R32G32B32A32_SFLOAT,
			Vulkan::getInstance()->GetWidth(),
			Vulkan::getInstance()->GetHeight(),
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		Draw::textureManager->CreateResource("GBuffer_normals", VK_FORMAT_R8G8B8A8_UNORM,
			Vulkan::getInstance()->GetWidth(),
			Vulkan::getInstance()->GetHeight(),
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		Draw::textureManager->CreateResource("GBuffer_albedo", VK_FORMAT_R8G8B8A8_UNORM,
			Vulkan::getInstance()->GetWidth(),
			Vulkan::getInstance()->GetHeight(),
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		Draw::textureManager->CreateResource("GBuffer_metallic_roughness", VK_FORMAT_R16G16B16A16_SFLOAT,
			Vulkan::getInstance()->GetWidth(),
			Vulkan::getInstance()->GetHeight(),
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		Draw::textureManager->CreateDepthResource("GBuffer_depth");

		// SSAO
		Draw::textureManager->CreateTextureFromData(
			"ssaoNoiseMap",
			Gloable::SSAO::ssaoNoise.data(), 
			Gloable::SSAO::ssaoNoise.size() * sizeof(glm::vec4), 
			VK_FORMAT_R32G32B32A32_SFLOAT, 
			Gloable::SSAO::SSAO_NOISE_DIM, 
			Gloable::SSAO::SSAO_NOISE_DIM, 
			VK_FILTER_NEAREST);

		Draw::textureManager->CreateResource("ssaoMap", 
			VK_FORMAT_R8_UNORM, Vulkan::getInstance()->GetWidth(), Vulkan::getInstance()->GetHeight(),
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);


		RenderPass* rp = new RenderPass();
		rp->CreateRenderPass();
		nameToRenderPass[RenderPassType::Default] = rp;
		
		RenderPass* rp2 = new RenderPass();
		rp2->CreateOffScreenRenderPass("brdf_lut");
		nameToRenderPass[RenderPassType::BRDFLUT] = rp2;

		RenderPass* rp3 = new RenderPass();
		rp3->CreateOffScreenRenderPass("irradiance_attachment", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		nameToRenderPass[RenderPassType::IRRADIANCE] = rp3;

		RenderPass* rp4 = new RenderPass();
		rp4->CreateOffScreenRenderPass("prefilter_attachment", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		nameToRenderPass[RenderPassType::PREFILTER] = rp4;

		RenderPass* rp5 = new RenderPass();
		rp5->CreateDeferredRenderPass();
		nameToRenderPass[RenderPassType::DEFERRED] = rp5;

		RenderPass* rp6 = new RenderPass();
		rp6->CreateFullScreenRenderPass("ssaoMap");
		nameToRenderPass[RenderPassType::FULLSCREEN_SSAO] = rp6;
	}

	void DestroyRenderPass()
	{
		for (auto& rp : nameToRenderPass) {
			delete rp.second;
		}
	}

}