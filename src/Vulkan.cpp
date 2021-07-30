#include "Vulkan.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace Graphics {

	static char* concatenate_strings(uint32_t string_count, const char* const* strings) {
		size_t output_size = 1;
		for (uint32_t i = 0; i != string_count; ++i) {
			output_size += strlen(strings[i]);
		}
		char* result = (char*)malloc(output_size);
		size_t output_length = 0;
		for (uint32_t i = 0; i != string_count; ++i) {
			size_t length = strlen(strings[i]);
			memcpy(result + output_length, strings[i], sizeof(char) * length);
			output_length += length;
		}
		result[output_length] = 0;
		return result;
	}
	Vulkan::Vulkan()
	{
	}

	Vulkan::~Vulkan()
	{
	}

	void Vulkan::BeginFrame()
	{
		const char application_display_name[] = "Vulkan renderer";
		const char application_internal_name[] = "vulkan_renderer";

		if (create_vulkan_device(&device, application_internal_name, 0, VK_FALSE)) {
			exit(1);
		}
		if (create_or_resize_swapchain(&swapchain, &device, VK_FALSE, application_display_name, width, height, VK_FALSE)) {
			exit(1);
		}

		glfwSetWindowSize(swapchain.window, width, height);
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForVulkan(swapchain.window, true);
	}

	void Vulkan::EndFrame()
	{
	}

	int Vulkan::create_vulkan_device(device_t* device, const char* application_internal_name, uint32_t physical_device_index, VkBool32 request_ray_tracing)
	{
		// Clear the object
		memset(device, 0, sizeof(device_t));
		// Initialize GLFW
		if (!glfwInit()) {
			printf("GLFW initialization failed.\n");
			return 1;
		}
		// Create a Vulkan instance

		VkApplicationInfo application_info = {};
		application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		application_info.pApplicationName = application_internal_name;
		application_info.pEngineName = application_internal_name;
		application_info.applicationVersion = 100;
		application_info.engineVersion = 100;
		application_info.apiVersion = VK_MAKE_VERSION(1, 2, 0);

		uint32_t surface_extension_count;
		const char** surface_extension_names = glfwGetRequiredInstanceExtensions(&surface_extension_count);
		device->instance_extension_count = surface_extension_count;
		device->instance_extension_names = (const char**)malloc(sizeof(char*) * device->instance_extension_count);
		for (uint32_t i = 0; i != surface_extension_count; ++i)
			device->instance_extension_names[i] = surface_extension_names[i];
		const char* layer_names[] = { "VK_LAYER_KHRONOS_validation" };

		VkInstanceCreateInfo instance_create_info = {};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pApplicationInfo = &application_info;
		instance_create_info.enabledExtensionCount = device->instance_extension_count;
		instance_create_info.ppEnabledExtensionNames = device->instance_extension_names;
#ifdef NDEBUG
		instance_create_info.enabledLayerCount = 0;
#else
		instance_create_info.enabledLayerCount = COUNT_OF(layer_names);
#endif
		instance_create_info.ppEnabledLayerNames = layer_names;

		VkResult result;
		if (result = vkCreateInstance(&instance_create_info, device->allocator, &device->instance)) {
			printf("Failed to create a Vulkan instance (error code %d) with the following extensions and layers:\n", result);
			for (uint32_t i = 0; i != instance_create_info.enabledExtensionCount; ++i)
				printf("%s\n", instance_create_info.ppEnabledExtensionNames[i]);
			for (uint32_t i = 0; i != instance_create_info.enabledLayerCount; ++i)
				printf("%s\n", instance_create_info.ppEnabledLayerNames[i]);
			printf("Please check that Vulkan is installed properly and try again. Or try running the release build, which disables validation layers.\n");
			destroy_vulkan_device(device);
			return 1;
		}
		// Figure out what physical device should be used
		if (vkEnumeratePhysicalDevices(device->instance, &device->physical_device_count, NULL)) {
			printf("Failed to enumerate physical devices (e.g. GPUs) to be used with Vulkan.\n");
			destroy_vulkan_device(device);
			return 1;
		}
		device->physical_devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * device->physical_device_count);
		if (vkEnumeratePhysicalDevices(device->instance, &device->physical_device_count, device->physical_devices)) {
			destroy_vulkan_device(device);
			return 1;
		}
		printf("The following physical devices (GPUs) are available to Vulkan:\n");
		for (uint32_t i = 0; i != device->physical_device_count; ++i) {
			VkPhysicalDeviceProperties device_properties;
			vkGetPhysicalDeviceProperties(device->physical_devices[i], &device_properties);
			printf("%u - %s%s\n", i, device_properties.deviceName, (i == physical_device_index) ? " (used)" : "");
			if (i == physical_device_index) {
				device->physical_device_properties = device_properties;
			}
		}
		if (physical_device_index >= device->physical_device_count) {
			printf("The physical device with index %u is to be used but does not exist.\n", physical_device_index);
			destroy_vulkan_device(device);
			return 1;
		}
		device->physical_device = device->physical_devices[physical_device_index];
		// Enumerate available memory types
		vkGetPhysicalDeviceMemoryProperties(device->physical_device, &device->memory_properties);
		// Learn about available queues
		vkGetPhysicalDeviceQueueFamilyProperties(device->physical_device, &device->queue_family_count, NULL);
		if (!device->queue_family_count) {
			printf("No Vulkan queue family available.\n");
			destroy_vulkan_device(device);
			return 1;
		}
		device->queue_family_properties = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * device->queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device->physical_device, &device->queue_family_count, device->queue_family_properties);
		// Pick a queue that supports graphics and compute
		uint32_t required_queue_flags = VK_QUEUE_GRAPHICS_BIT & VK_QUEUE_COMPUTE_BIT;
		for (device->queue_family_index = 0;
			device->queue_family_index < device->queue_family_count
			&& (device->queue_family_properties[device->queue_family_index].queueFlags & required_queue_flags);
			++device->queue_family_index)
		{
		}
		if (device->queue_family_index == device->queue_family_count) {
			printf("No Vulkan queue family supports graphics and compute.\n");
			destroy_vulkan_device(device);
			return 1;
		}
		// Figure out whether ray queries are supported
		if (request_ray_tracing) {
			uint32_t extension_count = 0;
			vkEnumerateDeviceExtensionProperties(device->physical_device, NULL, &extension_count, NULL);
			VkExtensionProperties* extensions = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extension_count);
			if (vkEnumerateDeviceExtensionProperties(device->physical_device, NULL, &extension_count, extensions))
				extension_count = 0;
			for (uint32_t i = 0; i != extension_count; ++i)
				if (strcmp(extensions[i].extensionName, VK_KHR_RAY_QUERY_EXTENSION_NAME) == 0)
					device->ray_tracing_supported = VK_TRUE;
			free(extensions);
		}
		// Select device extensions
		const char* base_device_extension_names[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
			VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME
		};
		const char* ray_tracing_device_extension_names[] = {
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			VK_KHR_RAY_QUERY_EXTENSION_NAME,
		};
		device->device_extension_count = COUNT_OF(base_device_extension_names);
		if (device->ray_tracing_supported)
			device->device_extension_count += COUNT_OF(ray_tracing_device_extension_names);
		device->device_extension_names = (const char**)malloc(sizeof(char*) * device->device_extension_count);
		for (uint32_t i = 0; i != COUNT_OF(base_device_extension_names); ++i)
			device->device_extension_names[i] = base_device_extension_names[i];
		if (device->ray_tracing_supported)
			for (uint32_t i = 0; i != COUNT_OF(ray_tracing_device_extension_names); ++i)
				device->device_extension_names[COUNT_OF(base_device_extension_names) + i] = ray_tracing_device_extension_names[i];
		// Create a device
		float queue_priorities[1] = { 0.0f };


		VkDeviceQueueCreateInfo queue_info = {};
		queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		queue_info.queueCount = 1;
		queue_info.pQueuePriorities = queue_priorities;
		queue_info.queueFamilyIndex = device->queue_family_index;

		VkPhysicalDeviceFeatures enabled_features = {};
		enabled_features.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
		enabled_features.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo device_info = {};
		device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_info.pNext = NULL;
		device_info.queueCreateInfoCount = 1;
		device_info.pQueueCreateInfos = &queue_info;
		device_info.enabledExtensionCount = device->device_extension_count;
		device_info.ppEnabledExtensionNames = device->device_extension_names;
		device_info.pEnabledFeatures = &enabled_features;

		if (vkCreateDevice(device->physical_device, &device_info, device->allocator, &device->device)) {
			printf("Failed to create a Vulkan device with the following extensions:\n");
			for (uint32_t i = 0; i != device_info.enabledExtensionCount; ++i) {
				printf("%s\n", device_info.ppEnabledExtensionNames[i]);
			}
			destroy_vulkan_device(device);
			return 1;
		}
		// Create a command pool for each queue
		VkCommandPoolCreateInfo command_pool_info = {};
		command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_info.queueFamilyIndex = device->queue_family_index;
		command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(device->device, &command_pool_info, device->allocator, &device->command_pool)) {
			printf("Failed to create a command pool for a queue that supports graphics and compute.\n");
			destroy_vulkan_device(device);
			return 1;
		}
		// Grab the selected queue
		vkGetDeviceQueue(device->device, device->queue_family_index, 0, &device->queue);
		// Give feedback about ray tracing
		if (device->ray_tracing_supported)
			printf("Ray tracing is available.\n");
		else if (request_ray_tracing)
			printf("Ray tracing was requested but is unavailable. Try installing the latest GPU drivers or using a different physical device.\n");

	}

	void Vulkan::destroy_vulkan_device(device_t* device)
	{
		if (device->command_pool) vkDestroyCommandPool(device->device, device->command_pool, device->allocator);
		free(device->queue_family_properties);
		if (device->device) vkDestroyDevice(device->device, device->allocator);
		free(device->physical_devices);
		if (device->instance) vkDestroyInstance(device->instance, device->allocator);
		free(device->instance_extension_names);
		free(device->device_extension_names);
		glfwTerminate();
		// Mark the object as cleared
		memset(device, 0, sizeof(*device));
	}
	void Vulkan::partially_destroy_old_swapchain(swapchain_t* swapchain, const device_t* device) {
		if (swapchain->image_views)
			for (uint32_t i = 0; i != swapchain->image_count; ++i)
				vkDestroyImageView(device->device, swapchain->image_views[i], device->allocator);
		free(swapchain->image_views);
		free(swapchain->images);
		free(swapchain->present_modes);
		free(swapchain->surface_formats);
		// Mark the object as cleared except for swapchain and window
		swapchain_t cleared = {};
		cleared.window = swapchain->window;
		cleared.swapchain = swapchain->swapchain;
		cleared.surface = swapchain->surface;

		(*swapchain) = cleared;
	}
	int Vulkan::create_or_resize_swapchain(swapchain_t* swapchain, const device_t* device, VkBool32 resize, const char* application_display_name, uint32_t width, uint32_t height, VkBool32 use_vsync)
	{
		swapchain_t old_swapchain = { 0 };
		if (resize) {
			partially_destroy_old_swapchain(swapchain, device);
			old_swapchain = *swapchain;
		}
		memset(swapchain, 0, sizeof(*swapchain));
		// Create a window
		if (resize) {
			swapchain->window = old_swapchain.window;
			old_swapchain.window = NULL;
		}
		else {
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			swapchain->window = glfwCreateWindow(width, height, application_display_name, NULL, NULL);
			if (!swapchain->window) {
				printf("Window creation with GLFW failed.\n");
				destroy_swapchain(&old_swapchain, device);
				destroy_swapchain(swapchain, device);
				return 1;
			}
		}
		// Create a surface for the swap chain
		if (resize) {
			swapchain->surface = old_swapchain.surface;
			old_swapchain.surface = NULL;
		}
		else {
			if (glfwCreateWindowSurface(device->instance, swapchain->window, device->allocator, &swapchain->surface)) {
				const char* error_message = NULL;
				glfwGetError(&error_message);
				printf("Failed to create a surface:\n%s\n", error_message);
				destroy_swapchain(&old_swapchain, device);
				destroy_swapchain(swapchain, device);
				return 1;
			}
		}
		// Abort if the surface and the chosen queue does not support presentation
		VkBool32 presentation_supported;
		if (vkGetPhysicalDeviceSurfaceSupportKHR(device->physical_device, device->queue_family_index, swapchain->surface, &presentation_supported)
			|| presentation_supported == VK_FALSE)
		{
			printf("Failed to ascertain that the used surface supports presentation on screen.\n");
			destroy_swapchain(&old_swapchain, device);
			destroy_swapchain(swapchain, device);
			return 1;
		}
		// Determine an appropriate surface format
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical_device, swapchain->surface, &swapchain->surface_format_count, NULL)) {
			printf("Failed to query available surface formats.\n");
			destroy_swapchain(&old_swapchain, device);
			destroy_swapchain(swapchain, device);
			return 1;
		}
		swapchain->surface_formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * swapchain->surface_format_count);
		swapchain->format = VK_FORMAT_UNDEFINED;
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical_device, swapchain->surface, &swapchain->surface_format_count, swapchain->surface_formats)) {
			printf("Failed to query available surface formats.\n");
			destroy_swapchain(&old_swapchain, device);
			destroy_swapchain(swapchain, device);
			return 1;
		}
		if (swapchain->surface_format_count == 1 && swapchain->surface_formats[0].format == VK_FORMAT_UNDEFINED)
			swapchain->format = VK_FORMAT_B8G8R8A8_UNORM;
		for (uint32_t i = 0; i != swapchain->surface_format_count; ++i) {
			if (swapchain->surface_formats[i].colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				continue;
			VkFormat format = swapchain->surface_formats[i].format;
			if (format == VK_FORMAT_R8G8B8A8_UNORM || format == VK_FORMAT_R8G8B8A8_SRGB
				|| format == VK_FORMAT_B8G8R8A8_UNORM || format == VK_FORMAT_B8G8R8A8_SRGB
				|| format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 || format == VK_FORMAT_A2R10G10B10_UNORM_PACK32)
			{
				swapchain->format = format;
			}
		}
		if (swapchain->format == VK_FORMAT_UNDEFINED) {
			printf("Unable to determine an appropriate surface format. Only R8G8B8A8, B8G8R8A8, A2R10G10B10 or A2B10G10R10 formats are supported.\n");
			destroy_swapchain(swapchain, device);
			return 1;
		}
		// Query surface capabilities and present modes
		VkSurfaceCapabilitiesKHR surface_capabilities;
		if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physical_device, swapchain->surface, &surface_capabilities)) {
			printf("Failed to query surface capabilities of the physical device.\n");
			destroy_swapchain(&old_swapchain, device);
			destroy_swapchain(swapchain, device);
			return 1;
		}
		swapchain->present_mode_count;
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(device->physical_device, swapchain->surface, &swapchain->present_mode_count, NULL)) {
			printf("Failed to query presentation modes of the physical device.\n");
			destroy_swapchain(&old_swapchain, device);
			destroy_swapchain(swapchain, device);
			return 1;
		}
		swapchain->present_modes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * swapchain->present_mode_count);
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(device->physical_device, swapchain->surface, &swapchain->present_mode_count, swapchain->present_modes)) {
			printf("Failed to query presentation modes of the physical device.\n");
			destroy_swapchain(&old_swapchain, device);
			destroy_swapchain(swapchain, device);
			return 1;
		}
		int window_width = 0, window_height = 0;
		glfwGetFramebufferSize(swapchain->window, &window_width, &window_height);
		swapchain->extent.width = (surface_capabilities.currentExtent.width != 0xFFFFFFFF) ? surface_capabilities.currentExtent.width : window_width;
		swapchain->extent.height = (surface_capabilities.currentExtent.height != 0xFFFFFFFF) ? surface_capabilities.currentExtent.height : window_height;
		if (swapchain->extent.width * swapchain->extent.height == 0) {
			destroy_swapchain(&old_swapchain, device);
			return 2;
		}
		if (width != swapchain->extent.width || height != swapchain->extent.height)
			printf("The swapchain resolution is %ux%u.\n", swapchain->extent.width, swapchain->extent.height);
		// Find a supported composite alpha mode (one of these is guaranteed to be
		// set)
		VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		VkCompositeAlphaFlagBitsKHR composite_alpha_flags[4] = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (uint32_t i = 0; i < COUNT_OF(composite_alpha_flags); ++i) {
			if (surface_capabilities.supportedCompositeAlpha & composite_alpha_flags[i]) {
				composite_alpha = composite_alpha_flags[i];
				break;
			}
		}
		VkPresentModeKHR no_vsync_present_mode = VK_PRESENT_MODE_FIFO_KHR;
		for (uint32_t i = 0; i != swapchain->present_mode_count; ++i) {
			if (swapchain->present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR
				&& no_vsync_present_mode == VK_PRESENT_MODE_FIFO_KHR)
				no_vsync_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			if (swapchain->present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				no_vsync_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
		}
		if (no_vsync_present_mode == VK_PRESENT_MODE_FIFO_KHR)
			printf("No presentation mode without vertical synchronization is available. Enabling v-sync instead.\n");
		uint32_t requested_image_count = 2;
		if (requested_image_count < surface_capabilities.minImageCount)
			requested_image_count = surface_capabilities.minImageCount;
		if (requested_image_count > surface_capabilities.maxImageCount)
			requested_image_count = surface_capabilities.maxImageCount;


		VkSwapchainCreateInfoKHR swapchain_info = {};
		swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_info.surface = swapchain->surface;
		swapchain_info.minImageCount = requested_image_count;
		swapchain_info.imageFormat = swapchain->format;
		swapchain_info.imageExtent = swapchain->extent;
		swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchain_info.compositeAlpha = composite_alpha;
		swapchain_info.imageArrayLayers = 1;
		swapchain_info.presentMode = use_vsync ? VK_PRESENT_MODE_FIFO_KHR : no_vsync_present_mode;
		swapchain_info.clipped = VK_FALSE;
		swapchain_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		if (resize)
			swapchain_info.oldSwapchain = old_swapchain.swapchain;
		if (vkCreateSwapchainKHR(device->device, &swapchain_info, NULL, &swapchain->swapchain)) {
			const char* error_message = NULL;
			glfwGetError(&error_message);
			printf("Failed to create a swap chain. Vulkan reports:\n%s\n", error_message);
			destroy_swapchain(&old_swapchain, device);
			destroy_swapchain(swapchain, device);
			return 1;
		}
		destroy_swapchain(&old_swapchain, device);
		swapchain->image_count = 0;
		vkGetSwapchainImagesKHR(device->device, swapchain->swapchain, &swapchain->image_count, NULL);
		if (swapchain->image_count == 0) {
			printf("The created swap chain has no images.\n");
			destroy_swapchain(swapchain, device);
			return 1;
		}
		swapchain->images = (VkImage*)malloc(swapchain->image_count * sizeof(VkImage));
		if (vkGetSwapchainImagesKHR(device->device, swapchain->swapchain, &swapchain->image_count, swapchain->images)) {
			printf("Failed to retrieve swapchain images.\n");
			destroy_swapchain(swapchain, device);
			return 1;
		}
		swapchain->image_views = (VkImageView*)malloc(swapchain->image_count * sizeof(VkImageView));
		for (uint32_t i = 0; i < swapchain->image_count; i++) {
			VkImageViewCreateInfo color_image_view = {};
			color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			color_image_view.image = swapchain->images[i];
			color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
			color_image_view.format = swapchain->format;
			color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
			color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
			color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
			color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
			color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			color_image_view.subresourceRange.levelCount = 1;
			color_image_view.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device->device, &color_image_view, device->allocator, &swapchain->image_views[i])) {
				printf("Failed to create a view onto swapchain image %u.\n", i);
				destroy_swapchain(swapchain, device);
				return 1;
			}
		}
		return 0;
	}

	void Vulkan::destroy_swapchain(swapchain_t* swapchain, const device_t* device) {
		partially_destroy_old_swapchain(swapchain, device);
		if (swapchain->swapchain)
			vkDestroySwapchainKHR(device->device, swapchain->swapchain, device->allocator);
		if (swapchain->surface)
			vkDestroySurfaceKHR(device->instance, swapchain->surface, device->allocator);
		if (swapchain->window)
			glfwDestroyWindow(swapchain->window);
		// Mark the object as cleared
		memset(swapchain, 0, sizeof(*swapchain));
	}

	int Vulkan::find_memory_type(uint32_t* type_index, const device_t* device, uint32_t memory_type_bits, VkMemoryPropertyFlags property_mask) {
		for (uint32_t i = 0; i < device->memory_properties.memoryTypeCount; ++i) {
			if (memory_type_bits & (1 << i)) {
				if ((device->memory_properties.memoryTypes[i].propertyFlags & property_mask) == property_mask) {
					*type_index = i;
					return 0;
				}
			}
		}
		return 1;
	}

	const char* Vulkan::get_shader_stage_name(VkShaderStageFlags stage) {
		switch (stage) {
		case VK_SHADER_STAGE_VERTEX_BIT: return "vert";
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return "tesc";
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return "tese";
		case VK_SHADER_STAGE_GEOMETRY_BIT: return "geom";
		case VK_SHADER_STAGE_FRAGMENT_BIT: return "frag";
		case VK_SHADER_STAGE_COMPUTE_BIT: return "comp";
		case VK_SHADER_STAGE_RAYGEN_BIT_KHR: return "rgen";
		case VK_SHADER_STAGE_INTERSECTION_BIT_KHR: return "rint";
		case VK_SHADER_STAGE_ANY_HIT_BIT_KHR: return "rahit";
		case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR: return "rchit";
		case VK_SHADER_STAGE_MISS_BIT_KHR: return "rmiss";
		case VK_SHADER_STAGE_CALLABLE_BIT_KHR: return "rcall";
		case VK_SHADER_STAGE_TASK_BIT_NV: return "task";
		case VK_SHADER_STAGE_MESH_BIT_NV: return "mesh";
		default: return NULL;
		};
	}

	int Vulkan::compile_glsl_shader(shader_t* shader, const device_t* device, const shader_request_t* request) {
		if (!get_shader_stage_name(request->stage)) {
			printf("Invalid stage specification %u passed for shader %s.", request->stage, request->shader_file_path);
			return 1;
		}
		//char buf[80];
		//getcwd(buf, sizeof(buf));
		//printf("current working directory: %s\n", buf);

		// Verify that the shader file exists by opening and closing it
#ifndef NDEBUG
		FILE* shader_file = fopen(request->shader_file_path, "r");
		if (!shader_file) {
			printf("The shader file at path %s does not exist or cannot be opened.\n", request->shader_file_path);
			return 1;
		}
		fclose(shader_file);
#endif
		// Delete the prospective output file such that we can verify its existence
		// to see if the compiler did anything
		const char* spirv_path_pieces[] = { request->shader_file_path, ".spv" };
		char* spirv_path = concatenate_strings(COUNT_OF(spirv_path_pieces), spirv_path_pieces);
		remove(spirv_path);
		// Build the part of the command line for defines
		const char** define_pieces = (const char**)malloc(sizeof(char*) * 2 * request->define_count);
		for (uint32_t i = 0; i != request->define_count; ++i) {
			define_pieces[2 * i + 0] = " -D";
			define_pieces[2 * i + 1] = request->defines[i];
		}
		char* concatenated_defines = concatenate_strings(2 * request->define_count, define_pieces);
		free(define_pieces);
		// Construct the command line
		const char* command_line_pieces[] = {
			"glslangValidator -V100 --target-env spirv1.5 ",
			"-S ", get_shader_stage_name(request->stage),
	#ifndef NDEBUG
			" -g -Od ",
	#endif
			concatenated_defines,
			" -I\"", request->include_path, "\" ",
			"--entry-point ", request->entry_point,
			" -o \"", spirv_path,
			"\" \"", request->shader_file_path, "\""
		};
		char* command_line = concatenate_strings(COUNT_OF(command_line_pieces), command_line_pieces);
		free(concatenated_defines);
		// Check whether command processing is available at all
#ifndef NDEBUG
		if (!system(NULL)) {
			printf("No command processor is available. Cannot invoke the compiler to compile the shader at path %s.\n", request->shader_file_path);
			free(command_line);
			free(spirv_path);
			return 1;
		}
#endif
		// Invoke the command line and see whether it produced an output file
		system(command_line);
		FILE* file = fopen(spirv_path, "rb");
		if (!file) {
			printf("glslangValidator failed to compile the shader at path %s. The full command line is:\n%s\n", request->shader_file_path, command_line);
			free(command_line);
			free(spirv_path);
			return 1;
		}
		free(command_line);
		free(spirv_path);
		// Read the SPIR-V code from the file
		if (fseek(file, 0, SEEK_END) || (shader->spirv_size = ftell(file)) < 0) {
			printf("Failed to determine the file size for the compiled shader %s.", spirv_path);
			fclose(file);
			return 1;
		}
		shader->spirv_code = (uint32_t*)malloc(shader->spirv_size);
		fseek(file, 0, SEEK_SET);
		shader->spirv_size = fread(shader->spirv_code, sizeof(char), shader->spirv_size, file);
		fclose(file);
		// Create the Vulkan shader module
		VkShaderModuleCreateInfo module_info = {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = shader->spirv_size,
			.pCode = shader->spirv_code
		};
		if (vkCreateShaderModule(device->device, &module_info, device->allocator, &shader->module)) {
			printf("Failed to create a shader module from %s.\n", request->shader_file_path);
			destroy_shader(shader, device);
		}
		return 0;
	}

	void Vulkan::destroy_shader(shader_t* shader, const device_t* device) {
		if (shader->module) vkDestroyShaderModule(device->device, shader->module, device->allocator);
		free(shader->spirv_code);
		memset(shader, 0, sizeof(*shader));
	}

	int Vulkan::create_descriptor_sets(pipeline_with_bindings_t* pipeline, const device_t* device, const descriptor_set_request_t* request, uint32_t descriptor_set_count) {
		memset(pipeline, 0, sizeof(*pipeline));
		// Copy and complete the bindings
		VkDescriptorSetLayoutBinding* bindings = (VkDescriptorSetLayoutBinding*)malloc(sizeof(VkDescriptorSetLayoutBinding) * request->binding_count);
		for (uint32_t i = 0; i != request->binding_count; ++i) {
			bindings[i] = request->bindings[i];
			bindings[i].binding = i;
			bindings[i].stageFlags |= request->stage_flags;
			bindings[i].descriptorCount = (bindings[i].descriptorCount < request->min_descriptor_count)
				? request->min_descriptor_count
				: bindings[i].descriptorCount;
		}
		// Create the descriptor set layout
		VkDescriptorSetLayoutCreateInfo descriptor_layout_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = request->binding_count,
			.pBindings = bindings
		};
		if (vkCreateDescriptorSetLayout(device->device, &descriptor_layout_info, device->allocator, &pipeline->descriptor_set_layout)) {
			printf("Failed to create a descriptor set layout.\n");
			free(bindings);
			destroy_pipeline_with_bindings(pipeline, device);
			return 1;
		}

		// Create the pipeline layout using only this descriptor layout
		VkPipelineLayoutCreateInfo pipeline_layout_info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &pipeline->descriptor_set_layout
		};
		if (vkCreatePipelineLayout(device->device, &pipeline_layout_info, device->allocator, &pipeline->pipeline_layout)) {
			printf("Failed to create a pipeline layout from a single descriptor set layout.\n");
			free(bindings);
			destroy_pipeline_with_bindings(pipeline, device);
			return 1;
		}

		// Create a descriptor pool with the requested number of descriptor sets
		VkDescriptorPoolSize* descriptor_pool_sizes = (VkDescriptorPoolSize*)malloc(request->binding_count * sizeof(VkDescriptorPoolSize));
		for (uint32_t i = 0; i != request->binding_count; ++i) {
			descriptor_pool_sizes[i].type = bindings[i].descriptorType;
			descriptor_pool_sizes[i].descriptorCount = bindings[i].descriptorCount * descriptor_set_count;
		}
		free(bindings);
		VkDescriptorPoolCreateInfo descriptor_pool_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = descriptor_set_count,
			.poolSizeCount = request->binding_count,
			.pPoolSizes = descriptor_pool_sizes
		};
		if (vkCreateDescriptorPool(device->device, &descriptor_pool_info, device->allocator, &pipeline->descriptor_pool)) {
			printf("Failed to create a descriptor pool to allocate %u descriptor sets.\n", descriptor_set_count);
			free(descriptor_pool_sizes);
			destroy_pipeline_with_bindings(pipeline, device);
			return 1;
		}
		free(descriptor_pool_sizes);
		VkDescriptorSetLayout* layouts = (VkDescriptorSetLayout*)malloc(sizeof(VkDescriptorSetLayout) * descriptor_set_count);
		for (uint32_t i = 0; i != descriptor_set_count; ++i)
			layouts[i] = pipeline->descriptor_set_layout;
		VkDescriptorSetAllocateInfo descriptor_alloc_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = pipeline->descriptor_pool,
			.descriptorSetCount = descriptor_set_count,
			.pSetLayouts = layouts
		};
		pipeline->descriptor_sets = (VkDescriptorSet*)malloc(sizeof(VkDescriptorSet) * descriptor_set_count);
		if (vkAllocateDescriptorSets(device->device, &descriptor_alloc_info, pipeline->descriptor_sets)) {
			printf("Failed to create %u descriptor sets.\n", descriptor_alloc_info.descriptorSetCount);
			free(layouts);
			destroy_pipeline_with_bindings(pipeline, device);
			return 1;
		}
		free(layouts);
		return 0;
	}


	void Vulkan::complete_descriptor_set_write(uint32_t write_count, VkWriteDescriptorSet* writes, const descriptor_set_request_t* request) {
		for (uint32_t i = 0; i != write_count; ++i) {
			writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			if (writes[i].dstBinding < request->binding_count) {
				writes[i].descriptorType = request->bindings[writes[i].dstBinding].descriptorType;
				writes[i].descriptorCount = request->bindings[writes[i].dstBinding].descriptorCount;
			}
			writes[i].descriptorCount = (writes[i].descriptorCount < request->min_descriptor_count)
				? request->min_descriptor_count
				: writes[i].descriptorCount;
		}
	}


	void Vulkan::destroy_pipeline_with_bindings(pipeline_with_bindings_t* pipeline, const device_t* device) {
		if (pipeline->pipeline)
			vkDestroyPipeline(device->device, pipeline->pipeline, device->allocator);
		if (pipeline->descriptor_pool)
			vkDestroyDescriptorPool(device->device, pipeline->descriptor_pool, device->allocator);
		free(pipeline->descriptor_sets);
		if (pipeline->pipeline_layout)
			vkDestroyPipelineLayout(device->device, pipeline->pipeline_layout, device->allocator);
		if (pipeline->descriptor_set_layout)
			vkDestroyDescriptorSetLayout(device->device, pipeline->descriptor_set_layout, device->allocator);
		memset(pipeline, 0, sizeof(*pipeline));
	}

	int Vulkan::create_buffers(buffers_t* buffers, const VkBufferCreateInfo* buffer_infos, uint32_t buffer_count, VkMemoryPropertyFlags memory_properties) {

		memset(buffers, 0, sizeof(*buffers));
		buffers->buffer_count = buffer_count;
		if (buffer_count == 0)
			return 0;
		// Prepare the output data structure
		buffers->memory = NULL;
		buffers->buffers = (buffer_t*)malloc(sizeof(buffer_t) * buffers->buffer_count);
		memset(buffers->buffers, 0, sizeof(buffer_t) * buffers->buffer_count);
		// Create buffers
		VkMemoryAllocateFlags memory_allocate_flags = 0;
		for (uint32_t i = 0; i != buffers->buffer_count; ++i) {
			if (vkCreateBuffer(device.device, &buffer_infos[i], NULL, &buffers->buffers[i].buffer)) {
				printf("Failed to create a buffer of size %llu.\n", buffer_infos[i].size);
				destroy_buffers(buffers);
				return 1;
			}
			if (buffer_infos[i].usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
				memory_allocate_flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		}
		// Allocate memory with proper alignment
		VkDeviceSize current_size = 0;
		uint32_t memory_type_bits = 0xFFFFFFFF;
		for (uint32_t i = 0; i != buffers->buffer_count; ++i) {
			VkMemoryRequirements memory_requirements;
			vkGetBufferMemoryRequirements(device.device, buffers->buffers[i].buffer, &memory_requirements);
			memory_type_bits &= memory_requirements.memoryTypeBits;
			buffers->buffers[i].size = buffer_infos[i].size;
			buffers->buffers[i].offset = align_memory_offset(current_size, memory_requirements.alignment);
			current_size = buffers->buffers[i].offset + memory_requirements.size;
		}
		VkMemoryAllocateFlagsInfo allocation_flags = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
			.flags = memory_allocate_flags,
			.deviceMask = 1,
		};
		VkMemoryAllocateInfo allocation_info = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = (memory_allocate_flags != 0) ? &allocation_flags : NULL,
			.allocationSize = current_size
		};
		buffers->size = allocation_info.allocationSize;
		if (find_memory_type(&allocation_info.memoryTypeIndex, &device, memory_type_bits, memory_properties)) {
			printf("Failed to find an appropirate memory type for %u buffers with memory properties %u.\n", buffers->buffer_count, memory_properties);
			destroy_buffers(buffers);
			return 1;
		}
		if (vkAllocateMemory(device.device, &allocation_info, device.allocator, &buffers->memory)) {
			printf("Failed to allocate %llu bytes of memory for %u buffers.\n", allocation_info.allocationSize, buffers->buffer_count);
			destroy_buffers(buffers);
			return 1;
		}
		// Bind memory
		for (uint32_t i = 0; i != buffers->buffer_count; ++i) {
			if (vkBindBufferMemory(device.device, buffers->buffers[i].buffer, buffers->memory, buffers->buffers[i].offset)) {
				destroy_buffers(buffers);
				return 1;
			}
		}
		return 0;
	}

	void Vulkan::destroy_buffers(buffers_t* buffers) {

		for (uint32_t i = 0; i != buffers->buffer_count; ++i)
			if (buffers->buffers && buffers->buffers[i].buffer)
				vkDestroyBuffer(device.device, buffers->buffers[i].buffer, device.allocator);
		if (buffers->memory) vkFreeMemory(device.device, buffers->memory, device.allocator);
		free(buffers->buffers);
		memset(buffers, 0, sizeof(*buffers));
	}

}