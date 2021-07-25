//  Copyright (C) 2021, Christoph Peters, Karlsruhe Institute of Technology
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.


#pragma once
#include "vulkan_basics.h"
#include "camera.h"


/*! Holds boolean flags to indicate what aspects of the application need to be
	updated after a frame due to user input.*/
typedef struct application_updates_s {
	//! The application has just been started
	VkBool32 startup;
	//! The requested new dimensions for the content area of the window. Either
	//! one can be zero to indicate no change. Implies recreate_swapchain.
	uint32_t window_width, window_height;
	//! The window size has changed
	VkBool32 recreate_swapchain;
	//! All shaders need to be recompiled
	VkBool32 reload_shaders;
	//! The number of light sources in the scene or the number of vertices in a
	//! polygonal light source has changed
	VkBool32 update_light_count;
	//! A texture of a polygonal light source has changed
	VkBool32 update_light_textures;
	//! The scene itself has changed
	VkBool32 reload_scene;
	//! Settings that define how shading is performed have changed
	VkBool32 change_shading;
	//! The noise table needs to be recreated (usually transition between white
	//! and blue noise)
	VkBool32 regenerate_noise;
	//! The current camera and lights should be stored to / loaded from a file
	VkBool32 quick_save, quick_load;
} application_updates_t;

//! The sub pass that renders the user interface on top of the shaded frame
typedef struct interface_pass_s {
	//! Buffers holding all geometry for the interface pass. They are
	//! duplicated once per swapchain image.
	buffers_t geometry_allocation;
	//! A pointer to the buffers held by geometry_allocation with union type
	//! for convenient access by name. The array index is the swapchain index.
	union {
		struct {
			//! Vertex buffer for imgui geometry
			buffer_t vertices;
			//! Index buffer for imgui geometry
			buffer_t indices;
		};
		//! All buffers for the interface pass (same array as in
		//! geometry_allocation)
		buffer_t buffers[2];
	}*geometries;
	//! A pointer to the mapped memory of geometry_allocation
	void* geometry_data;
	//! The number of array entries in geometries, i.e. the number of swapchain
	//! images when this object was created
	uint32_t frame_count;

	//! geometry_count objects needed to query drawing commands for imgui
	// imgui_frame_t* frames;
	//! The image holding fonts and icons for imgui
	images_t texture;
	//! A graphics pipeline for rasterizing the user interface
	pipeline_with_bindings_t pipeline;
	//! The used vertex and fragment shader for rendering the user interface
	shader_t vertex_shader, fragment_shader;
	//! The sampler used to access the font texture of imgui
	VkSampler sampler;
} interface_pass_t;

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
	images_t targets_allocation;
	//! This union provides convenient access to all render targets, either as
	//! array or by name. The pointer is a pointer to an array of length
	//! duplicate_count.
	//union {
	//	struct {
	//		//! The depth buffer used in the geometry pass
	//		image_t depth_buffer;
	//		//! The visibility buffer, which stores a primitive index per pixel
	//		image_t visibility_buffer;
	//	};
	//	//! Array of all render targets available from this object
	//	image_t targets[2];
	//}*targets;

	image_t* targets;
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


typedef struct imgui_pass_s {
	pipeline_with_bindings_t pipeline;
} imgui_pass_t;

//! Gathers objects that are required to synchroinze rendering of a frame with
//! the swapchain
typedef struct frame_sync_s {
	//! Signaled once an image from the swapchain is available
	VkSemaphore image_acquired;
} frame_sync_t;


/*! Command buffers and synchronization objects for rendering and presenting a
	frame. Each instance is specific to one swapchain image.*/
typedef struct frame_workload_s {
	//! A command buffer using resources associated with the swapchain image to
	//! do all drawing, compute, ray tracing and transfer work for a frame
	VkCommandBuffer command_buffer;
	//! Whether this workload has ever been submitted for rendering
	VkBool32 used;
	//! Signaled once all drawing has finished. Used to synchronize CPU and GPU
	//! execution by waiting for this fence before reusing this workload
	VkFence drawing_finished_fence;
} frame_workload_t;

//! Handles a command buffer for each swapchain image and corresponding
//! synchronization objects
typedef struct frame_queue_s {
	//! Number of entries in the command_buffers array, i.e. the number of
	//! swapchain images when this object was created
	uint32_t frame_count;
	//! An array providing one workload to execute per swapchain image that may
	//! need to be rendered
	frame_workload_t* workloads;
	//! A ring buffer of synchronization objects. It is indexed by sync_index,
	//! not by swapchain image indices. It has size frame_count.
	frame_sync_t* syncs;
	//! Index of the most recent entry of syncs that was used for rendering
	uint32_t sync_index;
	//! Set if rendering of the previous frame encountered an exception that
	//! may indicate that the swapchain needs to be resized or if vsync is
	//! being switched
	VkBool32 recreate_swapchain;
} frame_queue_t;


typedef struct shading_pass_s {
	pipeline_with_bindings_t pipeline;
	shader_t vertex_shader, fragment_shader;
}shading_pass_t;

typedef struct point_light_pass_t {
	pipeline_with_bindings_t pipeline;
	shader_t vertex_shader, fragment_shader;
}light_pass_t;


typedef struct application_s {
	device_t device;
	swapchain_t swapchain;
	// imgui_handle_t imgui;
	shading_pass_t shading_pass;
	render_pass_t render_pass;
	render_targets_t render_targets;
	imgui_pass_t imgui_pass;
	point_light_pass_t point_light_pass;
	frame_queue_t frame_queue;
	VkClearValue        ClearValue;
	first_person_camera_t camera;
}application_t;

