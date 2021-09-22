#include "VulkanCore/Synchronization.h"

namespace Graphics
{
	Synchronization::Synchronization()
	{
		auto& device = Vulkan::getInstance()->device.device;
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		// VkResult res;
		presentComplete.resize(Vulkan::getInstance()->swapchain.image_count);
		renderComplete.resize(Vulkan::getInstance()->swapchain.image_count);

		for (size_t i = 0; i < Vulkan::getInstance()->swapchain.image_count; ++i) {

			if (vkCreateSemaphore(device, &semaphoreCreateInfo, Vulkan::getInstance()->device.allocator, &presentComplete[i]))
			{
				exit(1);
			}
			if (vkCreateSemaphore(device, &semaphoreCreateInfo, Vulkan::getInstance()->device.allocator, &renderComplete[i]))
			{
				exit(1);
			}
		}
		waitFences.resize(Vulkan::getInstance()->swapchain.image_count);
		imagesInFlight.resize(Vulkan::getInstance()->swapchain.image_count, VK_NULL_HANDLE);
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		for (auto& fence : waitFences) {
			vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);
		}

		// Create synchronization objects
		semaphoreCreateInfo = initializers::semaphoreCreateInfo();
		// Create a semaphore used to synchronize image presentation
		// Ensures that the image is displayed before we start submitting new commands to the queue
		vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete);
		// Create a semaphore used to synchronize command submission
		// Ensures that the image is not presented until all commands have been submitted and executed
		vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete);
		
	}
	Synchronization::~Synchronization()
	{
		for (size_t i = 0; i < Vulkan::getInstance()->swapchain.image_count; ++i) {
			vkDestroySemaphore(Vulkan::getInstance()->device.device, presentComplete[i], Vulkan::getInstance()->device.allocator);
			vkDestroySemaphore(Vulkan::getInstance()->device.device, renderComplete[i], Vulkan::getInstance()->device.allocator);
		}
		for (auto& fence : waitFences) {
			vkDestroyFence(Vulkan::getInstance()->device.device, fence, nullptr);
		}

		vkDestroySemaphore(Vulkan::getInstance()->device.device, semaphores.presentComplete, Vulkan::getInstance()->device.allocator);
		vkDestroySemaphore(Vulkan::getInstance()->device.device, semaphores.renderComplete, Vulkan::getInstance()->device.allocator);
	}
}