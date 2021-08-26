#pragma once
#ifndef COMMAND_POOL_H
#define COMMAND_POOL_H

#include "Graphics.h"
#include "Vulkan.h"
#include <memory>

namespace Graphics
{
	class CommandPool : public Graphics
	{
		friend class CommandBuffer;
	public:
		static CommandPool* getInstance() {
			if (instance == NULL) {
				instance = new CommandPool();
			}
			return instance;
		}
	private:
		CommandPool();
		~CommandPool();
	private:

		class Deletor {
		public:
			~Deletor() {
				if (CommandPool::instance != NULL)
					delete CommandPool::instance;
			}
		};
		static Deletor deletor;
		static CommandPool* instance;

		VkCommandPool cmdPool;
	};

}

#endif // !COMMAND_POOL_H
