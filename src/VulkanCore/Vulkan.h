#pragma once

#ifndef VULKAN_H
#define VULKAN_H

#include "Graphics.h"
#include <vector>
#include <memory>
#include <iostream>

namespace Draw
{
	class Drawable;
}

namespace RenderSystem
{
	class RenderLoop;
}

namespace Graphics 
{

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

	private:
		Vulkan(int width = 1280, int height = 760);
		~Vulkan();
		Vulkan(const Vulkan&) = delete;
		Vulkan& operator=(const Vulkan&) = delete;
	private:

		int create_vulkan_device(device_t* device, const char* application_internal_name, uint32_t physical_device_index, VkBool32 request_ray_tracing);


		void destroy_vulkan_device(device_t* device);

		int create_or_resize_swapchain(swapchain_t* swapchain, const device_t* device, VkBool32 resize,
			const char* application_display_name, uint32_t width, uint32_t height, VkBool32 use_vsync);

		void partially_destroy_old_swapchain(swapchain_t* swapchain, const device_t* device);

		//! Returns the aspect ratio, i.e. width / height for the given swapchain.
		static inline float get_aspect_ratio(const swapchain_t* swapchain) {
			return ((float)swapchain->extent.width) / ((float)swapchain->extent.height);
		}

		void destroy_swapchain(swapchain_t* swapchain, const device_t* device);

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

		int GetWidth()
		{
			return width;
		}

		int GetHeight()
		{
			return height;
		}

		swapchain_t& GetSwapchain()
		{
			return swapchain;
		}

		size_t GetSwapImageCount()
		{
			return swapchain.image_count;
		}

		static Vulkan* getInstance() {
			if (instance == NULL) {
				instance = new Vulkan();
			}
			return instance;
		}

	private:

		class Deletor {
		public:
			~Deletor() {
				std::cout << "Vulkan Deletor" << std::endl;
				if (Vulkan::instance != NULL)
					delete Vulkan::instance;
			}
		};
		static Deletor deletor;

		device_t device;
		swapchain_t swapchain;
		int width;
		int height;
		static Vulkan* instance;
	};
	
}

#endif // !VULKAN_H