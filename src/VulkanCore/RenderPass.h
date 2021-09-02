#ifndef RENDER_PASS_H
#define RENDER_PASS_H

#include "Vulkan.h"
#include "Image.h"
#include <memory>
#include <vector>

#include <string>
#include <map>
#include "Drawable/Texture.h"
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
		FULLSCREEN = 5,
	};

	class RenderPass : public Graphics
	{
		friend class Draw::Drawable;
		friend class CommandBuffer;

	public:
		~RenderPass();
		RenderPass() {}
		void CreateRenderPass();
		void CreateOffScreenRenderPass(string resource_name, VkImageLayout finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		void CreateDeferredRenderPass();
		void CreateFullScreenRenderPass();
	private:
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;		
	public:
		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> framebuffers;
		std::vector<VkClearValue> clearValues;
	};


	extern map<RenderPassType, RenderPass*> nameToRenderPass;

	void InitRenderPass();
	void DestroyRenderPass();
}
#endif // !RENDER_PASS_H
