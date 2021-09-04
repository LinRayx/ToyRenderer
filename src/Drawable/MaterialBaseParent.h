#ifndef MATERIAL_BASE_PARENT_H
#define MATERIAL_BASE_PARENT_H

#include "VulkanCore/vulkan_core_headers.h"

#include <map>
#include "DynamicVariable/DynamicConstant.h"
#include "Bindable/Shader.h"
#include "Drawable/Texture.h"
#include "Bindable/VertexBuffer.h"
#include "Bindable/IndexBuffer.h"
#include "Scene/Scene.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace Draw
{
	class MaterialBaseParent
	{
	public:
		MaterialBaseParent();
		template<typename T>
		void SetValue(std::string key1, std::string key2, T value)
		{
			if (bufs.count(key1) == 0) return;
			(*bufs[key1])[key2] = value;
		}

		template<typename T>
		void SetValue(std::string key1, std::string key2, size_t index, T value)
		{
			if (bufs.count(key1) == 0) return;
			(*bufs[key1])[key2][index] = value;
		}
		virtual void UpdateSceneData() {}
		virtual void Compile() {}
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd);
		virtual void Execute(shared_ptr<Graphics::CommandBuffer> cmdbuf_ptr) {}
		virtual bool SetUI() { return false;  }
		void Update(int cur);

		std::map<std::string, shared_ptr<Graphics::Buffer>> buffer_ptrs;
		std::map<std::string, shared_ptr<Dcb::Buffer>> bufs;
		std::shared_ptr<Graphics::DescriptorSetCore> desc_ptr;

	protected:
		void addLayout(std::string key, Dcb::RawLayout&& layout,
			Graphics::LayoutType layoutType, Graphics::DescriptorType descType, Graphics::StageFlag stage);
		void addTexture(Graphics::LayoutType layout_type, Graphics::StageFlag stage, VkImageView textureImageView, VkSampler textureSampler);

		void addTexture(Graphics::LayoutType layout_type, Graphics::StageFlag stage, VkImageView textureImageView, VkSampler textureSampler, uint32_t binding);

		void loadShader(Bind::ShaderType vert, Bind::ShaderType frag = Bind::ShaderType::EMPTY, VkSpecializationInfo specializationInfo = {});
		void loadVertexInfo();

		void addCubeTexture(string cube_texture_name);

		VkPipeline pipeline = VK_NULL_HANDLE;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = Graphics::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationState = Graphics::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
		VkPipelineColorBlendAttachmentState blendAttachmentState = Graphics::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
		VkPipelineColorBlendStateCreateInfo colorBlendState = Graphics::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
		VkPipelineDepthStencilStateCreateInfo depthStencilState = Graphics::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewportState = Graphics::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
		VkPipelineMultisampleStateCreateInfo multisampleState = Graphics::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState = Graphics::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
		VkViewport viewport;;
		VkRect2D scissor;

		VkPipelineViewportStateCreateInfo viewport_info;
		VkPipelineVertexInputStateCreateInfo vertexInputInfo;

		vector<VkPipelineShaderStageCreateInfo> shaderStages;
		shared_ptr<Bind::VertexBuffer> vBuffer_ptr = nullptr;
		shared_ptr<Bind::IndexBuffer> iBuffer_ptr = nullptr;
	};
}

#endif // !MATERIAL_BASE_PARENT_H
