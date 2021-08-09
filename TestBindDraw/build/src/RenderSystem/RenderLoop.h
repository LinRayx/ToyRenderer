#ifndef RENDER_LOOP_H
#define RENDER_LOOP_H
#include "Vulkan.h"
#include "Drawable.h"
#include "Camera.h"
#include <memory>
#include "FrameTimer.h"

namespace RenderSystem
{
	class RenderLoop
	{

	public:
		RenderLoop(std::shared_ptr<Graphics::Vulkan> vulkan_ptr, std::shared_ptr<Draw::Drawable> drawable_ptr, std::shared_ptr<Control::Camera> camera_ptr)
			: vulkan_ptr(vulkan_ptr), drawable_ptr(drawable_ptr), camera_ptr(camera_ptr)
		{
			frameT_ptr = std::make_shared<FrameTimer>();
		}

		void Loop()
		{
			while (vulkan_ptr->WindowShouldClose())
			{
				glfwPollEvents();
				if (glfwGetKey(vulkan_ptr->swapchain.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
					printf("Escape pressed. Shutting down.\n");
					break;
				}
				frameT_ptr->Record();
				camera_ptr->Control_camera(vulkan_ptr->swapchain.window, frameT_ptr->Get());
				drawable_ptr->Update();
				drawable_ptr->Submit();
			}
		}
	private:
		std::shared_ptr<Graphics::Vulkan> vulkan_ptr;
		std::shared_ptr<Draw::Drawable> drawable_ptr;
		std::shared_ptr<Control::Camera> camera_ptr;
		std::shared_ptr<FrameTimer> frameT_ptr;
	};
}

#endif // !RENDER_LOOP_H
