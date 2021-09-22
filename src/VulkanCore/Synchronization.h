#pragma once
#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include "Vulkan.h"
#include <vector>
#include <memory>
#include "VulkanCore/VulkanInitalizers.hpp"

namespace Graphics
{
	class Synchronization : public Graphics
	{
		friend class CommandQueue;
	public:
		Synchronization();
		~Synchronization();
	private:

		struct {
			// Swap chain image presentation
			VkSemaphore presentComplete;
			// Command buffer submission and execution
			VkSemaphore renderComplete;
		} semaphores;

		std::vector<VkSemaphore> presentComplete;
		std::vector<VkSemaphore> renderComplete;
		std::vector<VkFence> waitFences;
		std::vector<VkFence> imagesInFlight;
	};
}

#endif // !SYNCHRONIZATION_H