#include "RenderPass.h"
#include <stdexcept>

namespace Graphics {

	RenderPass::~RenderPass()
	{

	}

	void RenderPass::AddResource(std::string name, bool isDepth)
	{
		resources.emplace_back(ImageResource(name, isDepth));
	}

	void RenderPass::CreateRenderPass()
	{
		std::vector< VkAttachmentDescription > attachments;
		std::vector< VkAttachmentReference > ColorAttachmentRefs;
		VkAttachmentReference depthAttachmentRef{};
		for (size_t i = 0; i < resources.size(); ++i) {
			VkAttachmentDescription colorAttachment{};
			VkAttachmentReference colorAttachmentRef{};
			
			colorAttachment.format = resources[i].format;
			colorAttachment.samples = msaaSamples;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			if (resources[i].isDepth)
			{
				colorAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthAttachmentRef.attachment = i;
			}
			else {
				colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachmentRef.attachment = i;
				ColorAttachmentRefs.emplace_back(std::move(colorAttachmentRef));
			}
			
			attachments.emplace_back(std::move(colorAttachment));
		}
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = static_cast<uint32_t>(ColorAttachmentRefs.size());
		subpass.pColorAttachments = ColorAttachmentRefs.data();
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		// subpass.pResolveAttachments = &colorAttachmentResolveRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(vulkan_ptr->device.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

		framebuffers.resize(vulkan_ptr->swapchain.image_count);
		for (size_t i = 0; i < framebuffers.size(); i++) {

			std::vector<VkImageView> atts;

			for (size_t j = 0; j < resources.size(); ++j) {
				atts.emplace_back(resources[j].imageView);
			}

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(atts.size());
			framebufferInfo.pAttachments = atts.data();
			framebufferInfo.width = vulkan_ptr->swapchain.extent.width;
			framebufferInfo.height = vulkan_ptr->swapchain.extent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(vulkan_ptr->device.device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void RenderPass::createColorResources(ImageResource& resource)
	{
		resource.format = vulkan_ptr->swapchain.format;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (resource.isDepth)
		{
			resource.format = vulkan_ptr->findDepthFormat();
			usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		
		image_ptr->createImage(vulkan_ptr->swapchain.extent.width, vulkan_ptr->swapchain.extent.height, 1, msaaSamples, resource.format, VK_IMAGE_TILING_OPTIMAL, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, resource.image, resource.imageMemory);
		resource.imageView = image_ptr->createImageView(resource.image, resource.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}