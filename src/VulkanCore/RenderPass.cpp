#include "RenderPass.h"

namespace Graphics {

	RenderPass::~RenderPass()
	{
		destroy_render_targets(&render_tangets);
		destroy_render_pass(&renderpass);
	}


	//! Creates render targets and associated objects
	int RenderPass::create_render_targets(render_targets_t* targets) {
		auto& swapchain = vulkan_ptr->swapchain;
		auto& device = vulkan_ptr->device;
		memset(targets, 0, sizeof(*targets));
		VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
		if (swapchain.format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 || swapchain.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32)
			color_format = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
		Image::image_request_t image_requests[] = {
			{// depth buffer
				.image_info = {
					.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
					.imageType = VK_IMAGE_TYPE_2D,
					.format = VK_FORMAT_D24_UNORM_S8_UINT,
					.extent = {swapchain.extent.width, swapchain.extent.height, 1},
					.mipLevels = 1, .arrayLayers = 1, .samples = VK_SAMPLE_COUNT_1_BIT,
					.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
				},
				.view_info = {
					.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
					.viewType = VK_IMAGE_VIEW_TYPE_2D,
					.subresourceRange = {
						.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT
					}
				}
			},
		};
		targets->target_count = COUNT_OF(image_requests);
		targets->duplicate_count = swapchain.image_count;
		// Duplicate the requests per swapchain image
		Image::image_request_t* all_requests = (Image::image_request_t*)malloc(sizeof(image_requests) * targets->duplicate_count);
		for (uint32_t i = 0; i != targets->duplicate_count; ++i) {
			memcpy(all_requests + i * targets->target_count, image_requests, sizeof(image_requests));
		}
		if (image_ptr->create_images(&targets->targets_allocation, all_requests,
			targets->target_count * targets->duplicate_count, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT))
		{
			printf("Failed to create render targets.\n");
			free(all_requests);
			destroy_render_targets(targets);
			return 1;
		}
		free(all_requests);

		targets->targets = targets->targets_allocation.images;
		return 0;
	}

	//! Frees objects and zeros
	void RenderPass::destroy_render_targets(render_targets_t* render_targets) {
		image_ptr->destroy_images(&render_targets->targets_allocation);
		memset(render_targets, 0, sizeof(*render_targets));
	}

	int RenderPass::create_render_pass(render_pass_t* pass, const render_targets_t* render_targets)
	{
		auto& swapchain = vulkan_ptr->swapchain;
		auto& device = vulkan_ptr->device;
		memset(pass, 0, sizeof(*pass));
		// Create the render pass
		VkAttachmentDescription attachments[] = {
			{ // 0 - Swapchain image
				.format = swapchain.format,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			},
			{ // 1 - Depth buffer
				.format = render_targets->targets[0].image_info.format,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			},

		};
		VkAttachmentReference depth_reference = { .attachment = 1, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		VkAttachmentReference swapchain_output_reference = { .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		vector< VkSubpassDescription > subpasses;

		VkSubpassDescription subpasses1 = {};
		subpasses1.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses1.pDepthStencilAttachment = &depth_reference;
		subpasses1.colorAttachmentCount = 1;
		subpasses1.pColorAttachments = &swapchain_output_reference;

		VkSubpassDescription subpasses2 = {};
		subpasses2.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses2.pDepthStencilAttachment = &depth_reference;
		subpasses2.colorAttachmentCount = 1;
		subpasses2.pColorAttachments = &swapchain_output_reference;

		VkSubpassDescription subpasses3 = {};
		subpasses3.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses3.colorAttachmentCount = 1;
		subpasses3.pColorAttachments = &swapchain_output_reference;

		subpasses.push_back(subpasses1); subpasses.push_back(subpasses2); subpasses.push_back(subpasses3);

		VkSubpassDependency dependencies[] = {
			{ // Swapchain image has been acquired
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0,
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			},
			{
				.srcSubpass = 0,
				.dstSubpass = 1,
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			},
			{
				.srcSubpass = 1,
				.dstSubpass = 2,
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			},
		};
		VkRenderPassCreateInfo renderpass_info = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = COUNT_OF(attachments), .pAttachments = attachments,
			.subpassCount = static_cast<uint32_t>(subpasses.size()), .pSubpasses = subpasses.data(),
			.dependencyCount = COUNT_OF(dependencies), .pDependencies = dependencies
		};
		if (vkCreateRenderPass(device.device, &renderpass_info, NULL, &pass->render_pass)) {
			printf("Failed to create a render pass for the geometry pass.\n");
			destroy_render_pass(pass);
			return 1;
		}

		// Create one framebuffer per swapchain image
		VkImageView framebuffer_attachments[2];
		VkFramebufferCreateInfo framebuffer_info = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = pass->render_pass,
			.attachmentCount = COUNT_OF(framebuffer_attachments),
			.pAttachments = framebuffer_attachments,
			.width = swapchain.extent.width,
			.height = swapchain.extent.height,
			.layers = 1
		};
		pass->framebuffer_count = swapchain.image_count;
		pass->framebuffers = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * pass->framebuffer_count);
		memset(pass->framebuffers, 0, sizeof(VkFramebuffer) * pass->framebuffer_count);
		for (uint32_t i = 0; i != pass->framebuffer_count; ++i) {
			framebuffer_attachments[1] = render_targets->targets[i].view;
			framebuffer_attachments[0] = swapchain.image_views[i];
			if (vkCreateFramebuffer(device.device, &framebuffer_info, NULL, &pass->framebuffers[i])) {
				printf("Failed to create a framebuffer for the main render pass.\n");
				destroy_render_pass(pass);
				return 1;
			}
		}
		return 0;
	}

	void RenderPass::destroy_render_pass(render_pass_t* pass) {
		auto& device = vulkan_ptr->device;
		for (uint32_t i = 0; i != pass->framebuffer_count; ++i)
			if (pass->framebuffers[i])
				vkDestroyFramebuffer(device.device, pass->framebuffers[i], NULL);
		free(pass->framebuffers);
		if (pass->render_pass) vkDestroyRenderPass(device.device, pass->render_pass, NULL);
		memset(pass, 0, sizeof(*pass));
	}

}