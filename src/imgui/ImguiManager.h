#ifndef IMGUI_MANAGER_H
#define IMGUI_MANAGER_H

#include "VulkanCore/vulkan_core_headers.h"


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
		ImguiManager( );
		~ImguiManager();
		void Init();
		void UpLoadFont(VkCommandBuffer command_buffer, VkQueue g_Queue);
		void beginFrame();
		void endFrame();

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
 

		bool update = true;

		VkRenderPass renderPass;
		std::vector<VkFramebuffer> framebuffers;
		ImVec4 clear_color;
		VkClearValue clearValue;
		ImDrawData* draw_data;
	};
}

#endif // !IMGUI_MANAGER_H

