#ifndef IMGUI_MANAGER_H
#define IMGUI_MANAGER_H

#include "VulkanCore/vulkan_core_headers.h"
#include "Bindable/Shader.h"

#include <memory>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <glm/glm.hpp>

namespace GUI
{
	class ImguiManager
	{

	public:
		struct PushConstBlock {
			glm::vec2 scale;
			glm::vec2 translate;
		} pushConstBlock;

		ImguiManager( );
		~ImguiManager();

		
		void prepareResources();
		void preparePipeline(const VkPipelineCache pipelineCache, const VkRenderPass renderPass);

		bool update();
		void draw(shared_ptr<Graphics::CommandBuffer> cmd_ptr);
		void resize(uint32_t width, uint32_t height);
		void freeResources();

		void loadShaders();
		bool visible = true;
		bool updated = false;
		float scale = 1.0f;
	private:

		Graphics::BufferV2 vertexBuffer;
		Graphics::BufferV2 indexBuffer;
		int32_t vertexCount = 0;
		int32_t indexCount = 0;

		VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		uint32_t subpass = 0;
		shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr;
		VkDeviceMemory fontMemory = VK_NULL_HANDLE;
		VkImage fontImage = VK_NULL_HANDLE;
		VkImageView fontView = VK_NULL_HANDLE;
		VkSampler sampler;

		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorSet descriptorSet;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;

		std::vector<VkPipelineShaderStageCreateInfo> shaders;


	};
}

#endif // !IMGUI_MANAGER_H

