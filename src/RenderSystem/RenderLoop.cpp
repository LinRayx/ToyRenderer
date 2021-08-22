#include "RenderSystem/RenderLoop.h"

namespace RenderSystem
{

	RenderLoop::RenderLoop()
	{
		vulkan_ptr = make_shared<Graphics::Vulkan>();
		cmdPool_ptr = make_shared<Graphics::CommandPool>(vulkan_ptr);
		cmdBuf_ptr = make_shared<Graphics::CommandBuffer>(vulkan_ptr, cmdPool_ptr);
		sync_ptr = make_shared<Graphics::Synchronization>(vulkan_ptr);
		cmdQue_ptr = make_shared<Graphics::CommandQueue>(vulkan_ptr, sync_ptr);
		desc_pool_ptr = make_shared<Graphics::DescriptorPool>(vulkan_ptr);
		image_ptr = make_shared<Graphics::Image>(vulkan_ptr);
		scene_ptr = make_shared<Control::Scene>(vulkan_ptr, vulkan_ptr->width, vulkan_ptr->height);
		texture_ptr = make_shared<Draw::Texture>(vulkan_ptr, cmdBuf_ptr, image_ptr);

		frameT_ptr = std::make_shared<FrameTimer>();
	}

	RenderLoop::~RenderLoop()
	{
		for (size_t i = 0; i < pso_vecs.size(); ++i) {
			if (pso_vecs[i] != nullptr) {
				delete pso_vecs[i];
			}
		}
	}

	void RenderLoop::Init()
	{
		PhonePSO* phonePSO = new PhonePSO(vulkan_ptr, desc_pool_ptr, image_ptr);
		
		Draw::Model* model1 = new Draw::Model(vulkan_ptr, scene_ptr, desc_pool_ptr, texture_ptr, "../assets/gobber/GoblinX.obj");
		// Draw::Model* model2 = new Draw::Model(vulkan_ptr, scene_ptr, desc_pool_ptr, "../assets/plane.obj");
		phonePSO->Add(model1);
		// phonePSO->Add(model2);
		pso_vecs.emplace_back(phonePSO);
		
	}

}