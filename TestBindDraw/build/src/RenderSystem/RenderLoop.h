#ifndef RENDER_LOOP_H
#define RENDER_LOOP_H
#include "Vulkan.h"
#include "Drawable.h"
#include <memory>


namespace RenderSystem
{
	class RenderLoop
	{

	public:
		RenderLoop(std::shared_ptr<Graphics::Vulkan> vulkan_ptr, std::shared_ptr<Draw::Drawable> drawable_ptr)
			: vulkan_ptr(vulkan_ptr), drawable_ptr(drawable_ptr)
		{}

		void Loop()
		{
			while (vulkan_ptr->WindowShouldClose())
			{
				drawable_ptr->Submit();
			}
		}
	private:
		std::shared_ptr<Graphics::Vulkan> vulkan_ptr;
		std::shared_ptr<Draw::Drawable> drawable_ptr;
	};
}

#endif // !RENDER_LOOP_H
