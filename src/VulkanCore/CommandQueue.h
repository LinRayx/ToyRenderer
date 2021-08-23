#ifndef COMMAND_QUEUE_H
#define COMMAND_QUEUE_H

#include "Graphics.h"
#include "Vulkan.h"
#include <memory>
#include "Synchronization.h"
#include <iostream>
#include "CommandBuffer.h"

namespace Graphics
{
	class CommandQueue : public Graphics
	{
	public:
		CommandQueue(std::shared_ptr<Vulkan> vulkan_ptr, std::shared_ptr<Synchronization> sync_ptr) 
			: vulkan_ptr(vulkan_ptr), sync_ptr(sync_ptr) {}

		void SetCommandBuffer(std::shared_ptr<CommandBuffer> cmdBuf_ptr)
		{
			this->cmdBuf_ptr = cmdBuf_ptr;
		}

		void Submit()
		{
			drawFrame();
		}

		int GetCurImageIndex();


	private:
		void drawFrame();

	private:
		uint32_t currentFrame = 0;
		uint32_t imageIndex = 0;

		std::shared_ptr<Vulkan> vulkan_ptr;
		std::shared_ptr<Synchronization> sync_ptr;
		std::shared_ptr<CommandBuffer> cmdBuf_ptr;

	};
}

#endif // !COMMAND_QUEUE_H