#include "ImguiManager.h"

namespace GUI
{
    ImguiManager::ImguiManager(std::shared_ptr<Graphics::Vulkan> vulkan_ptr,
        std::shared_ptr<Graphics::DescriptorPool> desc_pool_ptr) : vulkan_ptr(vulkan_ptr), desc_pool_ptr(desc_pool_ptr)
    {
    }
    ImguiManager::~ImguiManager()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    void ImguiManager::Init()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForVulkan(vulkan_ptr->GetSwapchain().window, true);
        createRenderPass();
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = vulkan_ptr->GetDevice().instance;
		init_info.PhysicalDevice = vulkan_ptr->GetDevice().physical_device;
		init_info.Device = vulkan_ptr->GetDevice().device;
		init_info.QueueFamily = vulkan_ptr->GetDevice().queue_family_index;
		init_info.Queue = vulkan_ptr->GetDevice().queue;
		init_info.PipelineCache = NULL;
		init_info.DescriptorPool = desc_pool_ptr->GetPool();
		init_info.Allocator = vulkan_ptr->GetDevice().allocator;
		init_info.MinImageCount = vulkan_ptr->GetSwapchain().image_count;
		init_info.ImageCount = vulkan_ptr->GetSwapchain().image_count;
		init_info.CheckVkResultFn = check_vk_result;
		ImGui_ImplVulkan_Init(&init_info, renderPass);

        clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	}

    void ImguiManager::UpLoadFont(VkCommandBuffer command_buffer, VkQueue g_Queue)
    {
        // Upload Fonts
        {
            VkCommandBufferBeginInfo begin_info = {};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            VkResult err = vkBeginCommandBuffer(command_buffer, &begin_info);
            check_vk_result(err);

            ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

            VkSubmitInfo end_info = {};
            end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            end_info.commandBufferCount = 1;
            end_info.pCommandBuffers = &command_buffer;
            err = vkEndCommandBuffer(command_buffer);
            check_vk_result(err);
            err = vkQueueSubmit(g_Queue, 1, &end_info, VK_NULL_HANDLE);
            check_vk_result(err);

            err = vkDeviceWaitIdle(vulkan_ptr->GetDevice().device);
            check_vk_result(err);
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
    }

    void ImguiManager::beginFrame()
    {
        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    bool ImguiManager::GetData()
    {
        // Rendering
        ImGui::Render();
        draw_data = ImGui::GetDrawData();
        bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        clearValue.color.float32[0] = clear_color.x * clear_color.w;
        clearValue.color.float32[1] = clear_color.y * clear_color.w;
        clearValue.color.float32[2] = clear_color.z * clear_color.w;
        clearValue.color.float32[3] = clear_color.w;
        return is_minimized;
    }

    void ImguiManager::BuildCommandBuffer(std::shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr)
    {
        
        bool is_minimized = GetData();
        if (is_minimized) return;
        auto& drawCmdBuffers = cmdBuf_ptr->drawCmdBuffers;
        for (size_t i = 0; i < drawCmdBuffers.size(); i++) {

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = framebuffers[i];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = vulkan_ptr->GetSwapchain().extent;


            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearValue;

            vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            // Record dear imgui primitives into command buffer
            ImGui_ImplVulkan_RenderDrawData(draw_data, drawCmdBuffers[i]);

            vkCmdEndRenderPass(drawCmdBuffers[i]);
        }
    }

    void ImguiManager::createRenderPass()
    {
        // Create the Render Pass
        {
            VkAttachmentDescription attachment = {};
            attachment.format = vulkan_ptr->GetSwapchain().format;
            attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            VkAttachmentReference color_attachment = {};
            color_attachment.attachment = 0;
            color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            VkSubpassDescription subpass = {};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &color_attachment;
            VkSubpassDependency dependency = {};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            VkRenderPassCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            info.attachmentCount = 1;
            info.pAttachments = &attachment;
            info.subpassCount = 1;
            info.pSubpasses = &subpass;
            info.dependencyCount = 1;
            info.pDependencies = &dependency;
            VkResult err = vkCreateRenderPass(vulkan_ptr->GetDevice().device, &info, vulkan_ptr->GetDevice().allocator, &renderPass);
            check_vk_result(err);

            framebuffers.resize(vulkan_ptr->GetSwapchain().image_count);
            for (size_t i = 0; i < framebuffers.size(); i++) {
                std::vector<VkImageView> atts;

                atts.emplace_back(vulkan_ptr->GetSwapchain().image_views[i]);

                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPass;
                framebufferInfo.attachmentCount = static_cast<uint32_t>(atts.size());
                framebufferInfo.pAttachments = atts.data();
                framebufferInfo.width = vulkan_ptr->GetSwapchain().extent.width;
                framebufferInfo.height = vulkan_ptr->GetSwapchain().extent.height;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(vulkan_ptr->GetDevice().device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create framebuffer!");
                }
            }
        }
    }
}
