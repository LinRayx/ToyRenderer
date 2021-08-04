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
		CommandQueue(std::shared_ptr<Vulkan> vulkan_ptr, std::shared_ptr<CommandBuffer> cmdBuf_ptr, std::shared_ptr<Synchronization> sync_ptr) 
			: vulkan_ptr(vulkan_ptr), cmdBuf_ptr(cmdBuf_ptr),sync_ptr(sync_ptr) {}

		void SetCommandBuffer(std::shared_ptr<CommandBuffer> cmdBuf_ptr)
		{
			this->cmdBuf_ptr = cmdBuf_ptr;
		}

		void Submit()
		{
			drawFrame();
		}
	private:
		void drawFrame()
		{

			std::vector< VkSemaphore > waitSemaphores;
			std::vector< VkSemaphore > signalSemaphores;
			uint32_t imageIndex = 0;
			vkWaitForFences(vulkan_ptr->device.device, 1, &sync_ptr->waitFences[currentFrame], VK_TRUE, UINT64_MAX);
			
			VkResult result = vkAcquireNextImageKHR(vulkan_ptr->device.device, vulkan_ptr->swapchain.swapchain, UINT64_MAX, sync_ptr->presentComplete[currentFrame], VK_NULL_HANDLE, &imageIndex);

			if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
				exit(1);
			}
			else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
				std::cout << "failed to acquire swap chain image!" << std::endl;
				exit(1);
			}

			if (sync_ptr->imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
				vkWaitForFences(vulkan_ptr->device.device, 1, &sync_ptr->imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
			}
			sync_ptr->imagesInFlight[imageIndex] = sync_ptr->waitFences[currentFrame];

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;


			waitSemaphores.emplace_back(sync_ptr->presentComplete[currentFrame]);
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores.data();
			submitInfo.pWaitDstStageMask = waitStages;

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmdBuf_ptr->drawCmdBuffers[imageIndex];

			signalSemaphores.emplace_back(sync_ptr->renderComplete[currentFrame]);
			submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
			submitInfo.pSignalSemaphores = signalSemaphores.data();

			vkResetFences(vulkan_ptr->device.device, 1, &sync_ptr->waitFences[currentFrame]);

			if (vkQueueSubmit(vulkan_ptr->device.queue, 1, &submitInfo, sync_ptr->waitFences[currentFrame]) != VK_SUCCESS) {
				throw std::runtime_error("failed to submit draw command buffer!");
			}

			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());;
			presentInfo.pWaitSemaphores = signalSemaphores.data();

			VkSwapchainKHR swapChains[] = { vulkan_ptr->swapchain.swapchain };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;

			presentInfo.pImageIndices = &imageIndex;

			result = vkQueuePresentKHR(vulkan_ptr->device.queue, &presentInfo);

			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ) {

			}
			else if (result != VK_SUCCESS) {
				throw std::runtime_error("failed to present swap chain image!");
			}

			currentFrame = (currentFrame + 1) % vulkan_ptr->swapchain.image_count;
		}

	private:
		uint32_t currentFrame = 0;

		std::shared_ptr<Vulkan> vulkan_ptr;
		std::shared_ptr<Synchronization> sync_ptr;
		std::shared_ptr<CommandBuffer> cmdBuf_ptr;

	};
}

#endif // !COMMAND_QUEUE_H