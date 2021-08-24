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
		gui_ptr = make_shared<GUI::ImguiManager>(vulkan_ptr, desc_pool_ptr);

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
		PhonePSO* phonePSO = new PhonePSO(vulkan_ptr, desc_pool_ptr);
		
		Draw::Model* model1 = new Draw::Model(vulkan_ptr, scene_ptr, desc_pool_ptr, texture_ptr, "../assets/nanosuit/nanosuit.obj", "../assets/nanosuit/");
		// Draw::Model* model2 = new Draw::Model(vulkan_ptr, scene_ptr, desc_pool_ptr, "../assets/plane.obj");
		phonePSO->Add(model1);
		// phonePSO->Add(model2);
		pso_vecs.emplace_back(phonePSO);
		modelWindows.resize(10);
		gui_ptr->Init();
		gui_ptr->UpLoadFont(cmdBuf_ptr->drawCmdBuffers[0], vulkan_ptr->GetDevice().queue);
	}

	void RenderLoop::Loop()
	{
		for (size_t i = 0; i < pso_vecs.size(); ++i) {
			pso_vecs[i]->BuildPipeline();
		}

		while (vulkan_ptr->WindowShouldClose())
		{
			glfwPollEvents();
			if (glfwGetKey(vulkan_ptr->swapchain.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
				printf("Escape pressed. Shutting down.\n");
				break;
			}
			frameT_ptr->Record();

			scene_ptr->camera_ptr->Control_camera(vulkan_ptr->swapchain.window, frameT_ptr->Get());
			int imageIndex = cmdQue_ptr->GetCurImageIndex();

			gui_ptr->beginFrame();
			int cnt = 0;
			for (size_t i = 0; i < pso_vecs.size(); ++i) {
				auto models = pso_vecs[i]->GetModels();
				for (size_t j = 0; j < models.size(); ++j) {
					modelWindows[cnt++].SetModel(models[j]);
				}
			}
			cmdBuf_ptr->Begin();
			for (size_t i = 0; i < pso_vecs.size(); ++i) {
				pso_vecs[i]->BuildCommandBuffer(cmdBuf_ptr);
			}

			gui_ptr->BuildCommandBuffer(cmdBuf_ptr);
			cmdBuf_ptr->End();
			cmdQue_ptr->SetCommandBuffer(cmdBuf_ptr);

			for (size_t i = 0; i < pso_vecs.size(); ++i) {
				pso_vecs[i]->Update(imageIndex);
			}

			cmdQue_ptr->Submit();

		}
	}

}