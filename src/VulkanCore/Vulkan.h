#pragma once

#ifndef VULKAN_H
#define VULKAN_H



#include "Graphics.h"
#include <vector>

namespace Draw
{
	class Drawable;
}

namespace RenderSystem
{
	class RenderLoop;
}

namespace Graphics {

	class Vulkan : public Graphics{
		friend class RenderPass;
		friend class Image;
		friend class Buffer;
		friend class Pipeline;
		friend class CommandPool;
		friend class CommandBuffer;
		friend class Synchronization;
		friend class CommandQueue;
		friend class DescriptorPool;
		friend class DescriptorSetCore;
		friend class DescriptorSetLayout;
		friend class Draw::Drawable;
		friend class RenderSystem::RenderLoop;
	private:
		/*! This macro initializes a function pointer for the Vulkan function with the
		given name. It uses GLFW to find it. The surrounding scope must have a
		device_t* device with a valid instance. The identifier of the function
		pointer is the function name, prefixed by p.*/

		/*! Holds Vulkan objects that are created up to device creation. This includes
			the instance, the physical device and the device. It depends on the choice
			of extensions and devices but not on a window or a resolution.*/
		typedef struct device_s {
			//! Number of loaded extensions for the instance
			uint32_t instance_extension_count;
			//! Names of loaded extensions for the instance
			const char** instance_extension_names;
			//! Number of loaded extensions for the device
			uint32_t device_extension_count;
			//! Names of loaded extensions for the device
			const char** device_extension_names;
			//! Boolean indicating whether ray tracing is available with the device
			VkBool32 ray_tracing_supported;

			//! Number of available physical devices
			uint32_t physical_device_count;
			//! List of available physical devices
			VkPhysicalDevice* physical_devices;
			//! The physical device that is being used
			VkPhysicalDevice physical_device;
			//! Properties of the used physical device
			VkPhysicalDeviceProperties physical_device_properties;
			//! Information about memory available from the used physical device
			VkPhysicalDeviceMemoryProperties memory_properties;

			//! This object makes Vulkan functions available
			VkInstance instance;
			//! The Vulkan device object for the physical device above
			VkDevice device;

			//! Number of available queue families
			uint32_t queue_family_count;
			//! Properties for each available queue
			VkQueueFamilyProperties* queue_family_properties;
			//! Index of a queue family that supports graphics and compute
			uint32_t queue_family_index;
			//! A queue based on queue_family_index
			VkQueue queue;
			//! A command pool for queue
			VkCommandPool command_pool;

			VkAllocationCallbacks* allocator;
		} device_t;


		/*! Holds Vulkan objects that are related to the swapchain. This includes the
			swapchain itself, the window and image views for images in the swapchain.
			It depends on the device and is changed substantially whenever the
			resolution changes.*/
		typedef struct swapchain_s {
			//! The extent of the images in the swapchain in pixels (i.e. the
			//! resolution on screen)
			VkExtent2D extent;
			//! The window containing the swapchain
			GLFWwindow* window;
			//! A surface created within this window
			VkSurfaceKHR surface;
			//! Number of available surface formats for the surface above
			uint32_t surface_format_count;
			//! List of available surface formats for the surface above
			VkSurfaceFormatKHR* surface_formats;
			//! The format of the held images
			VkFormat format;
			//! Number of available presentation modes
			uint32_t present_mode_count;
			//! List of available presentation modes
			VkPresentModeKHR* present_modes;
			//! The swapchain created within the window. NULL if the window was
			//! minimized during the last resize.
			VkSwapchainKHR swapchain;
			//! Number of images in the swapchain
			uint32_t image_count;
			//! List of images in the swapchain
			std::vector<VkImage> images;
			
			//! An image view for each image of the swapchain
			std::vector<VkImageView> image_views;
		} swapchain_t;

	public:
		Vulkan(int width = 800, int height = 600);
		~Vulkan();

		void BeginFrame();
		void EndFrame();

	private:
		/*! Creates all Vulkan objects that are created up to device creation. This
			includes the instance, the physical device and the device. It depends on
			the choice of extensions and devices but not on a window or a resolution.
			\param device The output structure. The calling side has to invoke
				destroy_vulkan_device() if the call succeeded.
			\param application_internal_name The name of the application that is
				advertised to Vulkan.
			\param physical_device_index The index of the physical device that is to
				be used in the list produced by vkEnumeratePhysicalDevices().
			\param request_ray_tracing Whether you want a device that supports ray
				tracing. If the physical device, does not support it, device creation
				still succeeds. Check device->ray_tracing_supported for the outcome.
			\return 0 indicates success. Upon failure, device is zeroed.*/
		int create_vulkan_device(device_t* device, const char* application_internal_name, uint32_t physical_device_index, VkBool32 request_ray_tracing);

		/*! Destroys a device that has been created successfully by
			create_vulkan_device().
			\param device The device that is to be destroyed and zeroed.*/
		void destroy_vulkan_device(device_t* device);


		/*! Creates Vulkan objects that are related to the swapchain. This includes the
			swapchain itself, the window, various buffers and image views. It depends
			on the device and is changed substantially whenever the resolution changes.
			Upon resize, the window is kept and shortcuts are used.
			\param swapchain The output structure and for resizes the existing
				swapchain. The calling side has to invoke destroy_swapchain()
				if the call succeeded.
			\param device A successfully created device.
			\param resize Whether to perform a resize (VK_TRUE) or first
				initialization.
			\param application_display_name The display name of the application, which
				is used as Window title. Irrelevant for resize.
			\param width, height The dimensions of the client area of the created
				window and thus the swapchain. The window manager may not respect this
				request, i.e. the actual resolution may differ. Irrelevant for resize.
			\param use_vsync 1 to use vertical synchronization, 0 to render as fast as
				possible (always do this for profiling).
			\return 0 indicates success. 1 upon failure, in which case swapchain is
				destroyed. 2 to indicate that the window is minimized. Swapchain
				resize can be successful once that changes.*/
		int create_or_resize_swapchain(swapchain_t* swapchain, const device_t* device, VkBool32 resize,
			const char* application_display_name, uint32_t width, uint32_t height, VkBool32 use_vsync);

		void partially_destroy_old_swapchain(swapchain_t* swapchain, const device_t* device);

		//! Returns the aspect ratio, i.e. width / height for the given swapchain.
		static inline float get_aspect_ratio(const swapchain_t* swapchain) {
			return ((float)swapchain->extent.width) / ((float)swapchain->extent.height);
		}

		/*! Destroys a swapchain that has been created successfully by
			create_swapchain().
			\param swapchain The swapchain that is to be destroyed and zeroed.
			\param device The device that has been used to construct the swapchain.*/
		void destroy_swapchain(swapchain_t* swapchain, const device_t* device);


		/*! Goes through memory types available from device and identifies the lowest
			index that satisfies all given requirements.
			\param memory_type_bits A bit mask indicating which memory type indices are
				admissible. Available from VkMemoryRequirements.
			\param property_mask A combination of VkMemoryPropertyFlagBits.
			\return 0 if type_index was set to a valid reply, 1 if no compatible memory
				is available.*/
		int find_memory_type(uint32_t* type_index, const device_t* device, uint32_t memory_type_bits, VkMemoryPropertyFlags property_mask);

		/*! Returns the smallest number that is greater equal offset and a multiple of
			the given positive integer. Useful for memory alignment.*/
		static inline VkDeviceSize align_memory_offset(VkDeviceSize offset, VkDeviceSize alignment) {
			return ((offset + alignment - 1) / alignment) * alignment;
		}

public:

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	VkFormat findDepthFormat();
	bool WindowShouldClose();

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	public:
		device_t& GetDevice()
		{
			return device;
		}

		swapchain_t& GetSwapchain()
		{
			return swapchain;
		}

		size_t GetSwapImageCount()
		{
			return swapchain.image_count;
		}
	private:
		device_t device;
		swapchain_t swapchain;
		int width;
		int height;
	};
}

#endif // !VULKAN_H