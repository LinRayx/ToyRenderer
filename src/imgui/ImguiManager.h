#ifndef IMGUI_MANAGER_H
#define IMGUI_MANAGER_H

#include "VulkanCore/Vulkan.h"
#include "VulkanCore/DescriptorPool.h"
#include "VulkanCore/RenderPass.h"
#include "VulkanCore/CommandQueue.h"

#include <memory>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace GUI
{
	class ImguiManager
	{
	private:
		struct RenderPass {
			VkRenderPass renderPass;
		} wd;
	public:
		ImguiManager(std::shared_ptr<Graphics::Vulkan> vulkan_ptr,
		std::shared_ptr<Graphics::DescriptorPool> desc_pool_ptr );
		~ImguiManager();
		void Init();
		void UpLoadFont(VkCommandBuffer command_buffer, VkQueue g_Queue);
		void beginFrame();

		bool GetData();

		void BuildCommandBuffer(std::shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr);
	private:

		static void check_vk_result(VkResult err)
		{
			if (err == 0)
				return;
			fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
			if (err < 0)
				abort();
		}

		void createRenderPass();

		std::shared_ptr<Graphics::Vulkan> vulkan_ptr;
		std::shared_ptr<Graphics::DescriptorPool> desc_pool_ptr;	 

		bool update = true;

		VkRenderPass renderPass;
		std::vector<VkFramebuffer> framebuffers;
		ImVec4 clear_color;
		VkClearValue clearValue;
		ImDrawData* draw_data;
	};
}

#endif // !IMGUI_MANAGER_H

