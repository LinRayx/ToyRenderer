#include "RenderPass.h"
#include <stdexcept>
#include <array>

namespace Graphics {

	RenderPass::~RenderPass()
	{

	}

	void RenderPass::AddResource(std::string name, bool isDepth)
	{
		ImageResource res(name, isDepth);
		createColorResources(res);
		
		resources.emplace_back(res);
	}

	void RenderPass::CreateRenderPass()
	{
		/*
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

		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = vulkan_ptr->swapchain.format;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachments.emplace_back(std::move(colorAttachmentResolve));
		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = resources.size();
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		ColorAttachmentRefs.emplace_back(std::move(colorAttachmentResolveRef));

		clearValues.resize(attachments.size());
		for (auto it : clearValues) {
			it.color = { {1.f, 0.2f, 0.2f, 1.0f} };
			it.depthStencil = { 1.0f, 0 };
		}


		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = static_cast<uint32_t>(ColorAttachmentRefs.size());
		subpass.pColorAttachments = ColorAttachmentRefs.data();
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

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
		renderPassInfo.pDependencies = &dependency;*/

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = vulkan_ptr->swapchain.format;
		colorAttachment.samples = msaaSamples;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = vulkan_ptr->findDepthFormat();
		depthAttachment.samples = msaaSamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
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

		if (vkCreateRenderPass(vulkan_ptr->device.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

		framebuffers.resize(vulkan_ptr->swapchain.image_count);
		for (size_t i = 0; i < framebuffers.size(); i++) {
			std::vector<VkImageView> atts;

			//for (size_t j = 0; j < resources.size(); ++j) {
			//	atts.emplace_back(resources[j].imageView);
			//}
			atts.emplace_back(vulkan_ptr->swapchain.image_views[i]);
			atts.emplace_back(resources[0].imageView);

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
		if (resource.isDepth) {
			resource.imageView = image_ptr->createImageView(resource.image, resource.format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
		}
		else {
			resource.imageView = image_ptr->createImageView(resource.image, resource.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
	}

	map<string, RenderPass*> nameToRenderPass;

	void InitRenderPass(shared_ptr<Vulkan> vulkan_ptr, shared_ptr<Image> image_ptr)
	{
		RenderPass* rp = new RenderPass(vulkan_ptr, image_ptr);
		rp->AddResource("Depth", true);
		rp->CreateRenderPass();
		nameToRenderPass["default"] = rp;
	}

}