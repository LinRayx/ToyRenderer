#ifndef RENDER_PASS_H
#define RENDER_PASS_H

#include "Vulkan.h"
#include "Image.h"
#include <memory>
#include <vector>
using namespace std;

namespace Draw
{
	class Drawable;
}

namespace Graphics {

	class RenderPass : public Graphics
	{
		friend class Draw::Drawable;
	public:
		/*! Provides convenient access to all render targets used by this application,
		except for swapchain images. These render targets are duplicated per
		swapchain image, to enable overlapping execution of work between frames.*/
		typedef struct render_targets_s {
			//! The number of held render targets per swapchain image, i.e. the number
			//! of entries in the union below
			uint32_t target_count;
			//! The number of duplicates for each target, i.e. the number of swapchain
			//! images when the render targets were allocated
			uint32_t duplicate_count;
			//! Images for all allocated render targets (not including the swapchain)
			Image::images_t targets_allocation;

			Image::image_t* targets;
		} render_targets_t;

		//! The render pass that renders a complete frame
		typedef struct render_pass_s {
			//! Number of held framebuffers (= swapchain images)
			uint32_t framebuffer_count;
			//! A framebuffer per swapchain image with the depth buffer (0), the
			//! visibility buffer (1) and the swapchain image (2) attached
			VkFramebuffer* framebuffers;
			//! The render pass that encompasses all subpasses for rendering a frame
			VkRenderPass render_pass;
		} render_pass_t;

	public:
		RenderPass(shared_ptr<Vulkan> _vulkan_ptr, shared_ptr<Image> _image_ptr) : vulkan_ptr(_vulkan_ptr), image_ptr(_image_ptr) {}
		~RenderPass();
	private:
		int create_render_targets(render_targets_t* targets);
		int create_render_pass(render_pass_t* pass, const render_targets_t* render_targets);
		void destroy_render_targets(render_targets_t* render_targets);
		void destroy_render_pass(render_pass_t* pass);
	private:
		shared_ptr<Vulkan> vulkan_ptr;
		shared_ptr<Image> image_ptr;
		render_pass_t renderpass;
	};

}
#endif // !RENDER_PASS_H
