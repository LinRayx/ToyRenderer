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
		Synchronization(std::shared_ptr<Vulkan> vulkan_ptr) : vulkan_ptr(vulkan_ptr)
		{
			VkSemaphoreCreateInfo semaphoreCreateInfo = {};
			semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			// VkResult res;
			presentComplete.resize(vulkan_ptr->swapchain.image_count);
			renderComplete.resize(vulkan_ptr->swapchain.image_count);

			for (size_t i = 0; i < vulkan_ptr->swapchain.image_count; ++i) {

				if (vkCreateSemaphore(vulkan_ptr->device.device, &semaphoreCreateInfo, vulkan_ptr->device.allocator, &presentComplete[i]))
				{
					exit(1);
				}
				if (vkCreateSemaphore(vulkan_ptr->device.device, &semaphoreCreateInfo, vulkan_ptr->device.allocator, &renderComplete[i]))
				{
					exit(1);
				}
			}
			waitFences.resize(vulkan_ptr->swapchain.image_count);
			imagesInFlight.resize(vulkan_ptr->swapchain.image_count, VK_NULL_HANDLE);
			VkFenceCreateInfo fenceCreateInfo = {};
			fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			for (auto& fence : waitFences) {
				vkCreateFence(vulkan_ptr->device.device, &fenceCreateInfo, nullptr, &fence);
			}
		}
		~Synchronization()
		{
			for (size_t i = 0; i < vulkan_ptr->swapchain.image_count; ++i) {
				vkDestroySemaphore(vulkan_ptr->device.device, presentComplete[i], vulkan_ptr->device.allocator);
				vkDestroySemaphore(vulkan_ptr->device.device, renderComplete[i], vulkan_ptr->device.allocator);
			}
			for (auto& fence : waitFences) {
				vkDestroyFence(vulkan_ptr->device.device, fence, nullptr);
			}
		}
	private:
		std::vector<VkSemaphore> presentComplete;
		std::vector<VkSemaphore> renderComplete;
		std::vector<VkFence> waitFences;
		std::vector<VkFence> imagesInFlight;
		std::shared_ptr<Vulkan> vulkan_ptr;
	};
}

#endif // !SYNCHRONIZATION_H