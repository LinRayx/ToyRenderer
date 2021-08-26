#ifndef RENDER_PASS_H
#define RENDER_PASS_H

#include "Vulkan.h"
#include "Image.h"
#include <memory>
#include <vector>

#include <string>
#include <map>
using namespace std;

namespace Draw
{
	class Drawable;
}

namespace Graphics {

	class RenderPass : public Graphics
	{
		friend class Draw::Drawable;
		friend class CommandBuffer;

	public:
		~RenderPass();
		RenderPass() {}
		void AddResource(std::string name, bool isDepth = false);
		void CreateRenderPass();
	private:

		struct ImageResource
		{
			ImageResource(std::string name, bool isDepth) : name(name), isDepth(isDepth) {}
			std::string name;
			bool isDepth;
			VkImage image;
			VkDeviceMemory imageMemory;
			VkImageView imageView;
			VkFormat format;
		};

		void createColorResources(ImageResource& resource);
	private:
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		std::vector<ImageResource> resources;
		
	public:
		VkRenderPass renderPass;
		std::vector<VkFramebuffer> framebuffers;
		std::vector<VkClearValue> clearValues;
	};


	extern map<string, RenderPass*> nameToRenderPass;

	void InitRenderPass();
}
#endif // !RENDER_PASS_H
