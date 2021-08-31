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

			//for (size_t j = 0; j < resources.size(); ++j) {
			//	atts.emplace_back(resources[j].imageView);
			//}
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

		VkFramebuffer framebuffer;
		vkCreateFramebuffer(Vulkan::getInstance()->GetDevice().device,  &framebufferCI, nullptr, &framebuffer);
		framebuffers.emplace_back(std::move(framebuffer));
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
	}

	void DestroyRenderPass()
	{
		for (auto& rp : nameToRenderPass) {
			delete rp.second;
		}
	}

}