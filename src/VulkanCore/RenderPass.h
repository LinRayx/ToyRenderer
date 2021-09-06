#ifndef RENDER_PASS_H
#define RENDER_PASS_H

#include "Vulkan.h"
#include <memory>
#include <vector>

#include <string>
#include <map>
#include "VulkanCore/VulkanInitalizers.hpp"
#include "Utils/GloableClass.h"

using namespace std;

namespace Graphics {
	enum class RenderPassType
	{
		Default = 0,
		BRDFLUT = 1,
		IRRADIANCE = 2,
		PREFILTER = 3,
		DEFERRED = 4,
		FULLSCREEN_SSAO = 5,
		FULLSCREEN_BLUR = 6,
	};

	class RenderPass : public Graphics
	{
		friend class Draw::Drawable;
		friend class CommandBuffer;

	public:
		struct RpData
		{
			VkFormat format;
			VkImageView& view;
		};
		~RenderPass();
		RenderPass();
		void CreateRenderPass(VkImageView& depthView);
		void CreateOffScreenRenderPass(VkFormat format, VkImageView& view, int width, int height, VkImageLayout finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		void CreateDeferredRenderPass(vector<RpData>& data);
		void CreateFullScreenRenderPass(VkFormat format, VkImageView& view, int width, int height);
	private:
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;		
	public:
		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> framebuffers;
		std::vector<VkClearValue> clearValues;
		int width;
		int height;
	};


	extern map<RenderPassType, RenderPass*> nameToRenderPass;

	void DestroyRenderPass();
}
#endif // !RENDER_PASS_H
