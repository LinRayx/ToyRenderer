#ifndef COMMAND_QUEUE_H
#define COMMAND_QUEUE_H

#include "Graphics.h"
#include "Vulkan.h"
#include <memory>
#include "Synchronization.h"
#include <iostream>
#include "CommandBuffer.h"
#include "VulkanCore/VulkanInitalizers.hpp"

namespace Graphics
{
	class CommandQueue : public Graphics
	{
	public:
		CommandQueue(std::shared_ptr<Synchronization> sync_ptr) 
			:sync_ptr(sync_ptr) {
			submitInfo = initializers::submitInfo();
			submitInfo.pWaitDstStageMask = &submitPipelineStages;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &sync_ptr->semaphores.presentComplete;
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &sync_ptr->semaphores.renderComplete;
		}

		~CommandQueue();

		void SetCommandBuffer(std::shared_ptr<CommandBuffer> cmdBuf_ptr)
		{
			this->cmdBuf_ptr = cmdBuf_ptr;
		}

		void AddCommandBuffer(VkCommandBuffer& cmdBuf)
		{
			cmdBufs.emplace_back(cmdBuf);
		}

		void Submit()
		{
			drawFrame();
			cmdBufs.clear();
		}

		void FlushCommandBuffer(VkCommandBuffer commandBuffer);

		int GetCurImageIndex();

		uint32_t prepareFrame();
		void submitFrame();
		void draw();

	private:
		void drawFrame();
		void drawFrameSimple();

	private:
		uint32_t currentFrame = 0;
		uint32_t imageIndex = 0;

		std::shared_ptr<Synchronization> sync_ptr;
		std::shared_ptr<CommandBuffer> cmdBuf_ptr;
		std::vector<VkCommandBuffer> cmdBufs;
		VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo;
	};
}

#endif // !COMMAND_QUEUE_H