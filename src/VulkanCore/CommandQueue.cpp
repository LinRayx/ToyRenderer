#include "CommandQueue.h"

namespace Graphics
{
	void CommandQueue::FlushCommandBuffer(VkCommandBuffer commandBuffer)
	{
		VkSubmitInfo submitInfo = initializers::submitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		// Create fence to ensure that the command buffer has finished executing
		VkFenceCreateInfo fenceInfo = initializers::fenceCreateInfo(VK_FLAGS_NONE);
		VkFence fence;
		vkCreateFence(Vulkan::getInstance()->GetDevice().device, &fenceInfo, nullptr, &fence);
		// Submit to the queue
		vkQueueSubmit(Vulkan::getInstance()->GetDevice().queue, 1, &submitInfo, fence);
		// Wait for the fence to signal that command buffer has finished executing
		vkWaitForFences(Vulkan::getInstance()->GetDevice().device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT);
		vkDestroyFence(Vulkan::getInstance()->GetDevice().device, fence, nullptr);
		vkQueueWaitIdle(Vulkan::getInstance()->GetDevice().queue);
	}
	int CommandQueue::GetCurImageIndex()
	{
		imageIndex = 0;
		vkWaitForFences(Vulkan::getInstance()->device.device, 1, &sync_ptr->waitFences[currentFrame], VK_TRUE, UINT64_MAX);

		VkResult result = vkAcquireNextImageKHR(Vulkan::getInstance()->device.device, Vulkan::getInstance()->swapchain.swapchain, UINT64_MAX, sync_ptr->presentComplete[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
			exit(1);
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			std::cout << "failed to acquire swap chain image!" << std::endl;
			exit(1);
		}

		if (sync_ptr->imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(Vulkan::getInstance()->device.device, 1, &sync_ptr->imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		sync_ptr->imagesInFlight[imageIndex] = sync_ptr->waitFences[currentFrame];
		return imageIndex;
	}

	void CommandQueue::drawFrame()
	{
		std::vector< VkSemaphore > waitSemaphores;
		std::vector< VkSemaphore > signalSemaphores;

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;


		waitSemaphores.emplace_back(sync_ptr->presentComplete[currentFrame]);
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = cmdBufs.size();

		submitInfo.pCommandBuffers = cmdBufs.data();

		signalSemaphores.emplace_back(sync_ptr->renderComplete[currentFrame]);
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
		submitInfo.pSignalSemaphores = signalSemaphores.data();

		vkResetFences(Vulkan::getInstance()->device.device, 1, &sync_ptr->waitFences[currentFrame]);

		if (vkQueueSubmit(Vulkan::getInstance()->device.queue, 1, &submitInfo, sync_ptr->waitFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}
		vkQueueWaitIdle(Vulkan::getInstance()->device.queue);
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());;
		presentInfo.pWaitSemaphores = signalSemaphores.data();

		VkSwapchainKHR swapChains[] = { Vulkan::getInstance()->swapchain.swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		VkResult result = vkQueuePresentKHR(Vulkan::getInstance()->device.queue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {

		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		currentFrame = (currentFrame + 1) % Vulkan::getInstance()->swapchain.image_count;
	}
	void CommandQueue::drawFrameSimple()
	{
		VkResult result = vkAcquireNextImageKHR(Vulkan::getInstance()->device.device, Vulkan::getInstance()->swapchain.swapchain, UINT64_MAX, sync_ptr->presentComplete[currentFrame], VK_NULL_HANDLE, &imageIndex);
		VkSubmitInfo submitInfo;
		submitInfo.commandBufferCount = cmdBufs.size();
		submitInfo.pCommandBuffers = cmdBufs.data();
		vkQueueSubmit(Vulkan::getInstance()->device.queue, 1, &submitInfo, VK_NULL_HANDLE);
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = NULL;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &Vulkan::getInstance()->swapchain.swapchain;
		presentInfo.pImageIndices = &imageIndex;
		vkQueuePresentKHR(Vulkan::getInstance()->device.queue, &presentInfo);
		vkQueueWaitIdle(Vulkan::getInstance()->device.queue);
	}
}