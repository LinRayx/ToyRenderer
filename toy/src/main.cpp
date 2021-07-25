
#include "main.h"
#include "frame_timer.h"

#include "string_utilities.h"
#include "gui.h"

#include "cube.h"

#include <vector>

using namespace std;

#define NDEBUG

static int width = 800;
static int height = 600;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	// fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

struct transform_matrix {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
} ubo;

void destroy_scene(application_t* app) {
	destroy_buffers(&app->model_buffers.vertices, &app->device);
	destroy_buffers(&app->cube_transform.transform_ubo, &app->device);
}

int load_scene(model_buffers_t* model_buffers, const device_t* device) {
	memset(model_buffers, 0, sizeof(*model_buffers));
	VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = sizeof(cube_vertices[0]) * cube_vertices.size(),
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	
	if (create_buffers(&model_buffers->vertices, device, &bufferInfo, 1, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
		return 1;
	}
	if (vkMapMemory(device->device, model_buffers->vertices.memory, 0, model_buffers->vertices.size, 0, &model_buffers->vertices_data)) {
		return 1;
	}

	memcpy(model_buffers->vertices_data, cube_vertices.data(), (size_t)model_buffers->vertices.size);

}

int init_camera(first_person_camera_t* camera) {
	camera->Position = glm::vec3(0, 0, 3);
	camera->WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	camera->Front = glm::vec3(0.0f, 0.0f, -1.0f);
	camera->Zoom = 45.0f;
	camera->Yaw = -90.0f;
	camera->Pitch = 0.0f;
	updateCameraVectors(camera);
	return 0;
}

int load_transform(cube_transform_t* cube_transform, const device_t* device, const swapchain_t* swapchain, first_person_camera_t* camera) {
	memset(cube_transform, 0, sizeof(*cube_transform));
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(ubo);
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBufferCreateInfo* buffer_infos = (VkBufferCreateInfo*)malloc(sizeof(VkBufferCreateInfo) * swapchain->image_count);
	for (uint32_t i = 0; i != swapchain->image_count; ++i)
		buffer_infos[i] = bufferInfo;

	if (create_buffers(&cube_transform->transform_ubo, device, buffer_infos, swapchain->image_count, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
		return 1;
	}

	if (vkMapMemory(device->device, cube_transform->transform_ubo.memory, 0, cube_transform->transform_ubo.size, 0, &cube_transform->data)) {
		return 1;
	}
	free(buffer_infos);
	ubo.view = GetViewMatrix(camera);
	ubo.proj = GetProjectMatrix(camera, width, height);
	ubo.model = glm::mat4(1.0f);
	ubo.model = glm::rotate(ubo.model, glm::radians(45.f), glm::vec3(1.0f, 0.3f, 0.5f));
	memcpy(cube_transform->data, &ubo, sizeof(ubo));
	memcpy((char*)cube_transform->data + sizeof(ubo), &ubo, sizeof(ubo));
}

//! Frees objects and zeros
void destroy_render_targets(render_targets_t* render_targets, const device_t* device) {
	destroy_images(&render_targets->targets_allocation, device);
	memset(render_targets, 0, sizeof(*render_targets));
}

//! Creates render targets and associated objects
int create_render_targets(render_targets_t* targets, const device_t* device, const swapchain_t* swapchain) {
	memset(targets, 0, sizeof(*targets));
	VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
	if (swapchain->format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 || swapchain->format == VK_FORMAT_A2B10G10R10_UNORM_PACK32)
		color_format = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
	image_request_t image_requests[] = {
		{// depth buffer
			.image_info = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.imageType = VK_IMAGE_TYPE_2D,
				.format = VK_FORMAT_D24_UNORM_S8_UINT,
				.extent = {swapchain->extent.width, swapchain->extent.height, 1},
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
	targets->duplicate_count = swapchain->image_count;
	// Duplicate the requests per swapchain image
	image_request_t* all_requests = (image_request_t*)malloc(sizeof(image_requests) * targets->duplicate_count);
	for (uint32_t i = 0; i != targets->duplicate_count; ++i) {
		memcpy(all_requests + i * targets->target_count, image_requests, sizeof(image_requests));
	}
	if (create_images(&targets->targets_allocation, device, all_requests,
		targets->target_count * targets->duplicate_count, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT))
	{
		printf("Failed to create render targets.\n");
		free(all_requests);
		destroy_render_targets(targets, device);
		return 1;
	}
	free(all_requests);

	targets->targets = targets->targets_allocation.images;
	return 0;
}

//! Frees objects and zeros
void destroy_render_pass(render_pass_t* pass, const device_t* device) {
	for (uint32_t i = 0; i != pass->framebuffer_count; ++i)
		if (pass->framebuffers[i])
			vkDestroyFramebuffer(device->device, pass->framebuffers[i], NULL);
	free(pass->framebuffers);
	if (pass->render_pass) vkDestroyRenderPass(device->device, pass->render_pass, NULL);
	memset(pass, 0, sizeof(*pass));
}


//! Creates the render pass that renders a complete frame
int create_render_pass(render_pass_t* pass, const device_t* device, const swapchain_t* swapchain, const render_targets_t* render_targets) {
	memset(pass, 0, sizeof(*pass));
	// Create the render pass
	VkAttachmentDescription attachments[] = {
		{ // 0 - Swapchain image
			.format = swapchain->format,
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
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
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
	subpasses2.colorAttachmentCount = 1;
	subpasses2.pColorAttachments = &swapchain_output_reference;

	subpasses.push_back(subpasses1); subpasses.push_back(subpasses2);

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
		}
	};
	VkRenderPassCreateInfo renderpass_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = COUNT_OF(attachments), .pAttachments = attachments,
		.subpassCount = static_cast<uint32_t>(subpasses.size()), .pSubpasses = subpasses.data(),
		.dependencyCount = COUNT_OF(dependencies), .pDependencies = dependencies
	};
	if (vkCreateRenderPass(device->device, &renderpass_info, NULL, &pass->render_pass)) {
		printf("Failed to create a render pass for the geometry pass.\n");
		destroy_render_pass(pass, device);
		return 1;
	}
	
	// Create one framebuffer per swapchain image
	VkImageView framebuffer_attachments[2];
	VkFramebufferCreateInfo framebuffer_info = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = pass->render_pass,
		.attachmentCount = COUNT_OF(framebuffer_attachments),
		.pAttachments = framebuffer_attachments,
		.width = swapchain->extent.width,
		.height = swapchain->extent.height,
		.layers = 1
	};
	pass->framebuffer_count = swapchain->image_count;
	pass->framebuffers = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * pass->framebuffer_count);
	memset(pass->framebuffers, 0, sizeof(VkFramebuffer) * pass->framebuffer_count);
	for (uint32_t i = 0; i != pass->framebuffer_count; ++i) {
		framebuffer_attachments[1] = render_targets->targets[i].view;
		framebuffer_attachments[0] = swapchain->image_views[i];
		if (vkCreateFramebuffer(device->device, &framebuffer_info, NULL, &pass->framebuffers[i])) {
			printf("Failed to create a framebuffer for the main render pass.\n");
			destroy_render_pass(pass, device);
			return 1;
		}
	}
	return 0;
}

//! Frees objects and zeros
void destroy_shading_pass(shading_pass_s* pass, const device_t* device) {
	destroy_pipeline_with_bindings(&pass->pipeline, device);
	destroy_shader(&pass->vertex_shader, device);
	destroy_shader(&pass->fragment_shader, device);
	memset(pass, 0, sizeof(*pass));
}

void destroy_imgui_pass(imgui_pass_s* pass, const device_t* device) {
	destroy_pipeline_with_bindings(&pass->pipeline, device);
	memset(pass, 0, sizeof(*pass));
}

int create_imgui_pass(imgui_pass_s* pass, const device_t* device) {
	memset(pass, 0, sizeof(*pass));
	// Create Descriptor Pool
	{
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		VkResult err = vkCreateDescriptorPool(device->device, &pool_info, device->allocator, &pass->pipeline.descriptor_pool);
		check_vk_result(err);
	}
	return 0;
}

int create_shading_pass(shading_pass_s* pass, const device_t* device, const swapchain_t* swapchain, const render_pass_t* render_pass, 
	cube_transform_t* cube_tranform) {
	memset(pass, 0, sizeof(*pass));
	pipeline_with_bindings_t* pipeline = &pass->pipeline;

	VkDescriptorSetLayoutBinding layout_bindings = {};
	layout_bindings.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layout_bindings.descriptorCount = 1;

	descriptor_set_request_t set_request{};
	set_request.bindings = &layout_bindings;
	set_request.min_descriptor_count = 1;
	set_request.binding_count = 1;
	set_request.stage_flags = VK_SHADER_STAGE_VERTEX_BIT;

	if (create_descriptor_sets(pipeline, device, &set_request, swapchain->image_count)) {
		return 1;
	}

	VkDescriptorBufferInfo descriptor_buffer_info = {};
	descriptor_buffer_info.offset = 0;
	VkWriteDescriptorSet descriptor_set_write = {};
	descriptor_set_write.dstBinding = 0;
	descriptor_set_write.pBufferInfo = &descriptor_buffer_info;

	complete_descriptor_set_write(1, &descriptor_set_write, &set_request);

	for (uint32_t i = 0; i != swapchain->image_count; ++i) {
		descriptor_buffer_info.buffer = cube_tranform->transform_ubo.buffers[i].buffer;
		descriptor_buffer_info.range = cube_tranform->transform_ubo.buffers[i].size;
		descriptor_set_write.dstSet = pipeline->descriptor_sets[i];
		vkUpdateDescriptorSets(device->device, 1, &descriptor_set_write, 0, NULL);
	}



	// Compile a vertex and fragment shader
	shader_request_t vertex_shader_request = {
		.shader_file_path = "../src/shaders/cube.vert.glsl",
		.include_path = "../src/shaders",
		.entry_point = "main",
		.stage = VK_SHADER_STAGE_VERTEX_BIT
	};
	shader_request_t fragment_shader_request = {
		.shader_file_path = "../src/shaders/cube.frag.glsl",
		.include_path = "../src/shaders",
		.entry_point = "main",
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT
	};

	if (compile_glsl_shader_with_second_chance(&pass->vertex_shader, device, &vertex_shader_request)) {
		printf("Failed to compile the vertex shader for the geometry pass.\n");
		destroy_shading_pass(pass, device);
		return 1;
	}
	if (compile_glsl_shader_with_second_chance(&pass->fragment_shader, device, &fragment_shader_request)) {
		printf("Failed to compile the fragment shader for the geometry pass.\n");
		destroy_shading_pass(pass, device);
		return 1;
	}



	// Define the graphics pipeline state

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto bindingDescription = Cube_t::getBindingDescription();
	auto attributeDescriptions = Cube_t::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapchain->extent.width;
	viewport.height = (float)swapchain->extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = { .extent = swapchain->extent };

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
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {};
	depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_info.depthTestEnable = VK_TRUE;
	depth_stencil_info.depthWriteEnable = VK_TRUE;
	depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;

	VkPipelineShaderStageCreateInfo shader_stages[2] = {
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = pass->vertex_shader.module,
		.pName = "main"
	},
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = pass->fragment_shader.module,
		.pName = "main"
	}
	};

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shader_stages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewport_info;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDepthStencilState = &depth_stencil_info;
	pipelineInfo.layout = pipeline->pipeline_layout;
	pipelineInfo.renderPass = render_pass->render_pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	if (vkCreateGraphicsPipelines(device->device, NULL, 1, &pipelineInfo, NULL, &pass->pipeline.pipeline)) {
		printf("Failed to create a graphics pipeline for the geometry pass.\n");
		destroy_shading_pass(pass, device);
		return 1;
	}
	return 0;

}


//! Frees objects and zeros
void destroy_frame_sync(frame_sync_t* sync, const device_t* device) {
	if (sync->image_acquired) vkDestroySemaphore(device->device, sync->image_acquired, NULL);
	memset(sync, 0, sizeof(*sync));
}

//! Frees objects and zeros
void destroy_frame_queue(frame_queue_t* queue, const device_t* device) {
	for (uint32_t i = 0; i != queue->frame_count; ++i) {
		if (queue->workloads) {
			frame_workload_t* workload = &queue->workloads[i];
			if (workload->command_buffer)
				vkFreeCommandBuffers(device->device, device->command_pool, 1, &workload->command_buffer);
			if (workload->drawing_finished_fence)
				vkDestroyFence(device->device, workload->drawing_finished_fence, NULL);
		}
		if (queue->syncs)
			destroy_frame_sync(&queue->syncs[i], device);
	}
	free(queue->workloads);
	free(queue->syncs);
	memset(queue, 0, sizeof(*queue));
}

//! Creates synchronization objects for rendering a single frame
//! \return 0 on success.
int create_frame_sync(frame_sync_t* sync, const device_t* device) {
	memset(sync, 0, sizeof(*sync));
	VkSemaphoreCreateInfo semaphore_info = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	if (vkCreateSemaphore(device->device, &semaphore_info, NULL, &sync->image_acquired)) {
		printf("Failed to create a semaphore.\n");
		destroy_frame_sync(sync, device);
		return 1;
	}
	return 0;
}

//! Creates the frame queue, including both synchronization objects and
//! command buffers
int create_frame_queue(frame_queue_t* queue, const device_t* device, const swapchain_t* swapchain) {
	memset(queue, 0, sizeof(*queue));
	// Create synchronization objects
	queue->frame_count = swapchain->image_count;
	queue->syncs = (frame_sync_t*)malloc(sizeof(frame_sync_t) * queue->frame_count);
	memset(queue->syncs, 0, sizeof(frame_sync_t) * queue->frame_count);
	for (uint32_t i = 0; i != queue->frame_count; ++i) {
		if (create_frame_sync(&queue->syncs[i], device)) {
			destroy_frame_queue(queue, device);
			return 1;
		}
	}
	// Allocate command buffers for rendering the scene to each of the swapchain
	// images
	VkCommandBufferAllocateInfo command_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = device->command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};
	queue->workloads = (frame_workload_t*)malloc(sizeof(frame_workload_t) * queue->frame_count);
	memset(queue->workloads, 0, sizeof(frame_workload_t) * queue->frame_count);
	for (uint32_t i = 0; i != queue->frame_count; ++i) {
		if (vkAllocateCommandBuffers(device->device, &command_info, &queue->workloads[i].command_buffer)) {
			printf("Failed to allocate command buffers for rendering.\n");
			destroy_frame_queue(queue, device);
			return 1;
		}
		VkFenceCreateInfo fence_info = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		if (vkCreateFence(device->device, &fence_info, NULL, &queue->workloads[i].drawing_finished_fence)) {
			printf("Failed to create a fence.\n");
			destroy_frame_queue(queue, device);
			return 1;
		}
	}
	return 0;
}

/*! This function records commands for rendering a frame to the given swapchain
	image into the given command buffer
	\return 0 on success.*/
int record_render_frame_commands(VkCommandBuffer cmd, application_t* app, uint32_t swapchain_index, ImDrawData* draw_data) {
	const device_t* device = &app->device;
	// Start recording commands
	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	if (vkBeginCommandBuffer(cmd, &begin_info)) {
		printf("Failed to begin using a command buffer for rendering the scene.\n");
		return 1;
	}
	// Begin the render pass that renders the whole frame
	
	VkClearValue clear_values[] = {
		{.color = {.float32 = {clear_color.x, clear_color.y, clear_color.z, clear_color.w} } },
		{.depthStencil = {.depth = 1.0f}},
		
	};

	VkRenderPassBeginInfo render_pass_begin = {};

	render_pass_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin.renderPass = app->render_pass.render_pass;
	render_pass_begin.framebuffer = app->render_pass.framebuffers[swapchain_index];
	render_pass_begin.renderArea.offset = { 0, 0 };
	render_pass_begin.renderArea.extent = app->swapchain.extent;
	render_pass_begin.clearValueCount = COUNT_OF(clear_values);
	render_pass_begin.pClearValues = clear_values;
	vkCmdBeginRenderPass(cmd, &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, app->shading_pass.pipeline.pipeline);

	const VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(cmd, 0, 1, &app->model_buffers.vertices.buffers->buffer, offsets);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
		app->shading_pass.pipeline.pipeline_layout, 0, 1, &app->shading_pass.pipeline.descriptor_sets[swapchain_index], 0, NULL);
	vkCmdDraw(cmd, cube_vertices.size(), 1, 0, 0);

	vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);

	const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
	if (!is_minimized)
	{
		ImGui_ImplVulkan_RenderDrawData(draw_data, cmd);
	}

	// The frame is rendered completely
	vkCmdEndRenderPass(cmd);



	// Finish recording
	if (vkEndCommandBuffer(cmd)) {
		printf("Failed to end using a command buffer for rendering the scene.\n");
		return 1;
	}

	return 0;
}
int render_frame(application_t* app) {
	// Get synchronization objects
	frame_queue_t* queue = &app->frame_queue;
	queue->sync_index = (queue->sync_index + 1) % queue->frame_count;
	frame_sync_t* sync = &queue->syncs[queue->sync_index];
	// Acquire the next swapchain image
	uint32_t swapchain_index;
	if (vkAcquireNextImageKHR(app->device.device, app->swapchain.swapchain, UINT64_MAX, sync->image_acquired, NULL, &swapchain_index)) {
		printf("Failed to acquire the next image from the swapchain.\n");
		return 1;
	}
	frame_workload_t* workload = &queue->workloads[swapchain_index];
	// Perform GPU-CPU synchronization to be sure that resources that we are
	// going to overwrite now are no longer used for rendering
	if (workload->used) {
		VkResult fence_result;
		do {
			fence_result = vkWaitForFences(app->device.device, 1, &workload->drawing_finished_fence, VK_TRUE, 100000000);
		} while (fence_result == VK_TIMEOUT);
		if (fence_result != VK_SUCCESS) {
			printf("Failed to wait for rendering of a frame to finish.\n");
			return 1;
		}
		if (vkResetFences(app->device.device, 1, &workload->drawing_finished_fence)) {
			printf("Failed to reset a fence for reuse in upcoming frames.\n");
			return 1;
		}
	}
	workload->used = VK_TRUE;


	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();

	// Record the command buffer for rendering
	if (record_render_frame_commands(workload->command_buffer, app, swapchain_index, draw_data)) {
		printf("Failed to record a command buffer for rendering the scene.\n");
		return 1;
	}

	// Queue the command buffer for rendering
	VkPipelineStageFlags destination_stage_masks[] = { VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo render_submit_info = {};

	render_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	render_submit_info.commandBufferCount = 1;
	render_submit_info.pCommandBuffers = &workload->command_buffer;
	render_submit_info.waitSemaphoreCount = 1;
	render_submit_info.pWaitSemaphores = &sync->image_acquired;
	render_submit_info.pWaitDstStageMask = destination_stage_masks;
	
	if (vkQueueSubmit(app->device.queue, 1, &render_submit_info, workload->drawing_finished_fence)) {
		printf("Failed to submit the command buffer for rendering a frame to the queue.\n");
		return 1;
	}
	// Present the image in the window
	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.swapchainCount = 1,
		.pSwapchains = &app->swapchain.swapchain,
		.pImageIndices = &swapchain_index
	};
	VkResult present_result;
	if (present_result = vkQueuePresentKHR(app->device.queue, &present_info)) {
		printf("Failed to present the rendered frame to the window. Error code %d. Attempting a swapchain resize.\n", present_result);
		app->frame_queue.recreate_swapchain = VK_TRUE;
	}
	return 0;
}
int update_application(application_t* app) {

	glfwSetWindowSize(app->swapchain.window, (int)width, (int)height);
	application_updates_t updates = { VK_FALSE };
	load_scene(&app->model_buffers, &app->device);
	init_camera(&app->camera);
	load_transform(&app->cube_transform, &app->device, &app->swapchain, &app->camera);
	create_render_targets(&app->render_targets, &app->device, &app->swapchain);
	create_render_pass(&app->render_pass, &app->device, &app->swapchain, &app->render_targets);
	create_shading_pass(&app->shading_pass, &app->device, &app->swapchain, &app->render_pass, &app->cube_transform);
	create_imgui_pass(&app->imgui_pass, &app->device);
	create_frame_queue(&app->frame_queue, &app->device, &app->swapchain);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(app->swapchain.window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = app->device.instance;
	init_info.PhysicalDevice = app->device.physical_device;
	init_info.Device = app->device.device;
	init_info.QueueFamily = app->device.queue_family_index;
	init_info.Queue = app->device.queue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = app->imgui_pass.pipeline.descriptor_pool;
	init_info.Allocator = app->device.allocator;
	init_info.MinImageCount = 2;
	init_info.ImageCount = app->swapchain.image_count;
	init_info.CheckVkResultFn = check_vk_result;
	init_info.Subpass = 1;
	ImGui_ImplVulkan_Init(&init_info, app->render_pass.render_pass);

	// Upload Fonts
	{
		VkResult err;
		// Use any command queue

		VkCommandPool command_pool = app->device.command_pool;
		VkCommandBuffer command_buffer = app->frame_queue.workloads[0].command_buffer;

		err = vkResetCommandPool(app->device.device, command_pool, 0);
		check_vk_result(err);
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		err = vkBeginCommandBuffer(command_buffer, &begin_info);
		check_vk_result(err);

		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

		VkSubmitInfo end_info = {};
		end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		end_info.commandBufferCount = 1;
		end_info.pCommandBuffers = &command_buffer;
		err = vkEndCommandBuffer(command_buffer);
		check_vk_result(err);
		err = vkQueueSubmit(app->device.queue, 1, &end_info, VK_NULL_HANDLE);
		check_vk_result(err);

		err = vkDeviceWaitIdle(app->device.device);
		check_vk_result(err);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
	return 0;
}

int startup_application(application_t* app) {
	const char application_display_name[] = "Vulkan renderer";
	const char application_internal_name[] = "vulkan_renderer";

	if (create_vulkan_device(&app->device, application_internal_name, 0, VK_FALSE)) {
		
		return 1;
	}
	if (create_or_resize_swapchain(&app->swapchain, &app->device, VK_FALSE, application_display_name, width, height, VK_FALSE)) {
		
		return 1;
	}




	update_application(app);
	return 0;
}

int handle_frame_input(application_t* app) {
	record_frame_time();
	// Define the user interface for the current frame





	return 0;
}

//! Destroys all objects associated with this application. Probably the last
//! thing you invoke before shutdown.
void destroy_application(application_t* app) {
	if (app->device.device)
		vkDeviceWaitIdle(app->device.device);
	destroy_frame_queue(&app->frame_queue, &app->device);
	destroy_shading_pass(&app->shading_pass, &app->device);
	destroy_imgui_pass(&app->imgui_pass, &app->device);
	destroy_render_pass(&app->render_pass, &app->device);
	destroy_render_targets(&app->render_targets, &app->device);
	destroy_scene(app);
	
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	destroy_swapchain(&app->swapchain, &app->device);
	destroy_vulkan_device(&app->device);
}

int main() {
	application_t app;

	if (startup_application(&app)) {
		printf("Application startup has failed.\n");
		return 1;
	}

	while (!glfwWindowShouldClose(app.swapchain.window)) {
		glfwPollEvents();
		// Check whether the window is minimized
		if (app.swapchain.swapchain) {
			if (handle_frame_input(&app)) break;
			render_frame(&app);
		}
	}
	destroy_application(&app);

	return 0;
}