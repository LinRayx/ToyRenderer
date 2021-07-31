#include "Image.h"

namespace Graphics {

	Image::~Image()
	{
	}

	int Image::create_images(images_t* images, const image_request_t* requests, uint32_t image_count, VkMemoryPropertyFlags memory_properties)
	{
		auto& device = vulkan_ptr->device;
		// Mark the output cleared
		memset(images, 0, sizeof(*images));
		images->memory_properties = memory_properties;
		if (image_count == 0)
			// That was easy
			return 0;
		// Create the images
		images->images = (image_t*)malloc(sizeof(image_t) * image_count);
		memset(images->images, 0, sizeof(image_t) * image_count);
		images->image_count = image_count;
		for (uint32_t i = 0; i != image_count; ++i) {
			image_t* image = &images->images[i];
			image->image_info = requests[i].image_info;
			if (image->image_info.mipLevels == 0)
				image->image_info.mipLevels = Vulkan::get_mipmap_count_3d(image->image_info.extent);
			if (vkCreateImage(device.device, &image->image_info, device.allocator, &image->image)) {
				printf("Failed to create image %u.\n", i);
				print_image_requests(requests, image_count);
				destroy_images(images);
				return 1;
			}
		}
		// See which images prefer dedicated allocations
		uint32_t dedicated_count = 0;
		for (uint32_t i = 0; i != image_count; ++i) {
			VkMemoryDedicatedRequirements memory_dedicated_requirements = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS
			};
			VkMemoryRequirements2 memory_requirements_2 = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
				.pNext = &memory_dedicated_requirements
			};
			VkImageMemoryRequirementsInfo2 memory_requirements_info = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2,
				.image = images->images[i].image
			};
			vkGetImageMemoryRequirements2(device.device, &memory_requirements_info, &memory_requirements_2);
			images->images[i].dedicated_allocation = memory_dedicated_requirements.prefersDedicatedAllocation;
			if (images->images[i].dedicated_allocation == VK_TRUE)
				++dedicated_count;
		}
		// Get ready to allocate memory
		uint32_t shared_count = (dedicated_count == image_count) ? 0 : 1;
		images->memory_count = shared_count + dedicated_count;
		images->memories = (VkDeviceMemory*)malloc(sizeof(VkDeviceMemory) * images->memory_count);
		memset(images->memories, 0, sizeof(VkDeviceMemory) * images->memory_count);
		// Make all dedicated allocations and bind
		uint32_t allocation_index = shared_count;
		for (uint32_t i = 0; i != image_count; ++i) {
			image_t* image = &images->images[i];
			if (!image->dedicated_allocation)
				continue;
			VkMemoryRequirements memory_requirements;
			vkGetImageMemoryRequirements(device.device, image->image, &memory_requirements);
			VkMemoryDedicatedAllocateInfo dedicated_info = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
				.image = image->image
			};
			image->memory_size = memory_requirements.size;
			VkMemoryAllocateInfo memory_info = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = &dedicated_info,
				.allocationSize = memory_requirements.size
			};
			if (vulkan_ptr->find_memory_type(&memory_info.memoryTypeIndex, &device, memory_requirements.memoryTypeBits, images->memory_properties)) {
				printf("Failed to find an acceptable memory type for image %u.\n", i);
				print_image_requests(requests, image_count);
				destroy_images(images);
				return 1;
			}
			if (vkAllocateMemory(device.device, &memory_info, device.allocator, &images->memories[allocation_index])) {
				printf("Failed to allocate memory for image %u.\n", i);
				print_image_requests(requests, image_count);
				destroy_images(images);
				return 1;
			}
			// Bind memory
			image->memory_index = allocation_index;
			if (vkBindImageMemory(device.device, image->image, images->memories[allocation_index], 0)) {
				printf("Failed to bind memory for image %u.\n", i);
				print_image_requests(requests, image_count);
				destroy_images(images);
				return 1;
			}
			++allocation_index;
		}
		// Combine requirements for the shared allocation and figure out memory
		// offsets and allocation size
		uint32_t shared_memory_types = 0xFFFFFFFF;
		VkDeviceSize current_size = 0;
		for (uint32_t i = 0; i != image_count; ++i) {
			image_t* image = &images->images[i];
			if (image->dedicated_allocation)
				continue;
			VkMemoryRequirements memory_requirements;
			vkGetImageMemoryRequirements(device.device, image->image, &memory_requirements);
			image->memory_size = memory_requirements.size;
			shared_memory_types &= memory_requirements.memoryTypeBits;
			image->memory_offset = vulkan_ptr->align_memory_offset(current_size, memory_requirements.alignment);
			current_size = image->memory_offset + memory_requirements.size;
		}
		// Perform the shared allocation
		if (shared_count) {
			VkMemoryAllocateInfo memory_info = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.allocationSize = current_size
			};
			if (vulkan_ptr->find_memory_type(&memory_info.memoryTypeIndex, &device, shared_memory_types, images->memory_properties)) {
				printf("Failed to find an acceptable memory type for images sharing memory. Check your requests and consider using two separate pools.\n");
				print_image_requests(requests, image_count);
				destroy_images(images);
				return 1;
			}
			if (vkAllocateMemory(device.device, &memory_info, device.allocator, &images->memories[0])) {
				printf("Failed to allocate %llu bytes of memory for images sharing memory.\n", memory_info.allocationSize);
				print_image_requests(requests, image_count);
				destroy_images(images);
				return 1;
			}
			// Bind the memory
			for (uint32_t i = 0; i != image_count; ++i) {
				image_t* image = &images->images[i];
				if (image->dedicated_allocation)
					continue;
				image->memory_index = 0;
				if (vkBindImageMemory(device.device, image->image, images->memories[0], image->memory_offset)) {
					printf("Failed to bind memory for image %u.\n", i);
					print_image_requests(requests, image_count);
					destroy_images(images);
					return 1;
				}
			}
		}
		// Create the views
		for (uint32_t i = 0; i != image_count; ++i) {
			image_t* image = &images->images[i];
			image->view_info = requests[i].view_info;
			image->view_info.format = requests[i].image_info.format;
			image->view_info.image = image->image;
			if (image->view_info.subresourceRange.layerCount == 0)
				image->view_info.subresourceRange.layerCount = image->image_info.arrayLayers - image->view_info.subresourceRange.baseArrayLayer;
			if (image->view_info.subresourceRange.levelCount == 0)
				image->view_info.subresourceRange.levelCount = image->image_info.mipLevels - image->view_info.subresourceRange.baseMipLevel;
			if (image->view_info.sType == VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO) {
				if (vkCreateImageView(device.device, &image->view_info, device.allocator, &image->view)) {
					printf("Failed to create a view for image %u.\n", i);
					print_image_requests(requests, image_count);
					destroy_images(images);
					return 1;
				}
			}
		}
		return 0;
	}

	void Image::destroy_images(images_t* images)
	{
		auto& device = vulkan_ptr->device;
		// Destroy the individual images
		for (uint32_t i = 0; i != images->image_count; ++i) {
			image_t* image = &images->images[i];
			if (image->view) vkDestroyImageView(device.device, image->view, device.allocator);
			if (image->image) vkDestroyImage(device.device, image->image, device.allocator);
		}
		free(images->images);
		// Free the memory allocations
		for (uint32_t i = 0; i != images->memory_count; ++i) {
			if (images->memories[i])
				vkFreeMemory(device.device, images->memories[i], device.allocator);
		}
		free(images->memories);
		// Mark the object cleared
		memset(images, 0, sizeof(*images));
	}

	void Image::print_image_requests(const image_request_t* image_requests, uint32_t image_count) {
		printf("A description of each requested image follows:\n");
		for (uint32_t i = 0; i != image_count; ++i) {
			const VkImageCreateInfo* image_info = &image_requests[i].image_info;
			uint32_t mip_map_count = image_info->mipLevels;
			if (mip_map_count == 0)
				mip_map_count = Vulkan::get_mipmap_count_3d(image_info->extent);
			printf("%u: %ux%ux%u, %u layers, %u mipmaps, format %d.\n",
				i, image_info->extent.width, image_info->extent.height, image_info->extent.depth,
				image_info->arrayLayers, mip_map_count, image_info->format);
		}
	}

}