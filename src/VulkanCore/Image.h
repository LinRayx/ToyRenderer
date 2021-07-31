#pragma once
#ifndef IMAGE_H
#define IMAGE_H
#include "Vulkan.h"
#include <memory>
using namespace std;

namespace Graphics {

	class Image
	{
		friend class RenderPass;
	public:
		/*! This structure combines a Vulkan image object, with meta-data and the view.
		The memory allocation is handled elsewhere, typically by an images_t.*/
		typedef struct image_s {
			//! The creation info used to create image
			VkImageCreateInfo image_info;
			//! The creation info used to create view
			VkImageViewCreateInfo view_info;
			//! The Vulkan object for the image
			VkImage image;
			//! A view onto the contents of this image or NULL if no view was requested
			VkImageView view;
			//! The offset of this image within the used memory allocation
			VkDeviceSize memory_offset;
			//! The required size of the memory allocation for this image in bytes. It
			//! may be larger than the image data itself.
			VkDeviceSize memory_size;
			//! Non-zero iff this image has a dedicated memory allocation
			uint32_t dedicated_allocation;
			//! The index of the memory allocation that is bound to this image
			uint32_t memory_index;
		} image_t;


		/*! This object handles a list of Vulkan images among with the corresponding
			memory allocations.*/
		typedef struct images_s {
			//! Number of held images
			uint32_t image_count;
			//! The held images
			image_t* images;
			//! The number of used device memory allocations
			uint32_t memory_count;
			/*! The memory allocations used to store the images. The intent is that all
				images share one allocation, except for those which prefer dedicated
				allocatins.*/
			VkDeviceMemory* memories;
			/*! The memory properties that have to be suported for the memory
				allocations. Combination of VkMemoryPropertyFlagBits.*/
			VkMemoryPropertyFlags memory_properties;
		} images_t;

		/*! The information needed to request construction of an image.*/
		typedef struct image_request_s {
			/*! Complete image creation info. If the number of mip levels is set to
				zero, it will be automatically set using get_mipmap_count_3d().*/
			VkImageCreateInfo image_info;
			/*! Description of the view that is to be created. format and image do not
				need to be set. If the layer count or mip count are zero, they are set
				to match the corresponding values of the image. If sType is not
				VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, creation of a view is
				skipped.*/
			VkImageViewCreateInfo view_info;
		} image_request_t;

	public:
		Image(shared_ptr<Vulkan> _vulkan_ptr) : vulkan_ptr(_vulkan_ptr) {}
		~Image();
		/*! This function creates images according to all of the given requests,
		creates views for them and allocates memory. Images get dedicated memory if
		they prefer it. All other images share one allocation.
		\param images The result is written into this object. Upon success, the
			calling side is responsible for destroying it using
			destroy_images().
		\param device The used device.
		\param requests Pointer to descriptions of the requested images.
		\param image_count The number of requested images, matching the length of
			requests.
		\param memory_properties The memory flags that you want to enforce for the
			memory allocations. The most common choice would be
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT.
		\return 0 on success.*/
		int create_images(images_t* images,
			const image_request_t* requests, uint32_t image_count, VkMemoryPropertyFlags memory_properties);
		void destroy_images(images_t* images);
		void print_image_requests(const image_request_t* image_requests, uint32_t image_count);
	private:
		shared_ptr<Vulkan> vulkan_ptr;
	};
}

#endif // !IMAGE_H