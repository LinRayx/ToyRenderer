#pragma once
#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include "Vulkan.h"
#include <vector>
#include <memory>

namespace Graphics
{
	class Synchronization : public Graphics
	{
		friend class CommandQueue;
	public:
		Synchronization()
		{
			VkSemaphoreCreateInfo semaphoreCreateInfo = {};
			semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			// VkResult res;
			presentComplete.resize(Vulkan::getInstance()->swapchain.image_count);
			renderComplete.resize(Vulkan::getInstance()->swapchain.image_count);

			for (size_t i = 0; i < Vulkan::getInstance()->swapchain.image_count; ++i) {

				if (vkCreateSemaphore(Vulkan::getInstance()->device.device, &semaphoreCreateInfo, Vulkan::getInstance()->device.allocator, &presentComplete[i]))
				{
					exit(1);
				}
				if (vkCreateSemaphore(Vulkan::getInstance()->device.device, &semaphoreCreateInfo, Vulkan::getInstance()->device.allocator, &renderComplete[i]))
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
				vkCreateFence(Vulkan::getInstance()->device.device, &fenceCreateInfo, nullptr, &fence);
			}
		}
		~Synchronization()
		{
			for (size_t i = 0; i < Vulkan::getInstance()->swapchain.image_count; ++i) {
				vkDestroySemaphore(Vulkan::getInstance()->device.device, presentComplete[i], Vulkan::getInstance()->device.allocator);
				vkDestroySemaphore(Vulkan::getInstance()->device.device, renderComplete[i], Vulkan::getInstance()->device.allocator);
			}
			for (auto& fence : waitFences) {
				vkDestroyFence(Vulkan::getInstance()->device.device, fence, nullptr);
			}
		}
	private:
		std::vector<VkSemaphore> presentComplete;
		std::vector<VkSemaphore> renderComplete;
		std::vector<VkFence> waitFences;
		std::vector<VkFence> imagesInFlight;
	};
}

#endif // !SYNCHRONIZATION_H