
#pragma once
#ifndef BUFFER_H
#define BUFFER_H
#include "Vulkan.h"
#include <vector>
#include <memory>
#include "VulkanInitalizers.hpp"
#include <assert.h>
#include <map>
using namespace std;

namespace Graphics {


	class Buffer
	{
		friend class CommandBuffer;
		friend class DescriptorSetCore;
	public:
		Buffer(BufferUsage type, size_t size);
		Buffer(BufferUsage type, size_t size, void* data, size_t elem_count);
		~Buffer();
		void UpdateData(uint32_t currentImage, size_t size, const char* newData);
		static void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	private:
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		

		std::vector<VkDeviceMemory> buffersMemorys;
		VkBufferUsageFlags getUsage(BufferUsage type);
		size_t size;
		
		bool update;
	public:
		size_t elem_count;
		vector<VkBuffer> buffers;
	};

	struct BufferV2
	{
		VkDevice device;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDescriptorBufferInfo descriptor;
		VkDeviceSize size = 0;
		VkDeviceSize alignment = 0;
		void* mapped = nullptr;
		/** @brief Usage flags to be filled by external source at buffer creation (to query at some later point) */
		VkBufferUsageFlags usageFlags;
		/** @brief Memory property flags to be filled by external source at buffer creation (to query at some later point) */
		VkMemoryPropertyFlags memoryPropertyFlags;
		VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void unmap();
		VkResult bind(VkDeviceSize offset = 0);
		void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void copyTo(void* data, VkDeviceSize size);
		VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void destroy();

		static VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, BufferV2* buffer, VkDeviceSize size, void* data = nullptr);
	};

}
#endif // !BUFFER_H