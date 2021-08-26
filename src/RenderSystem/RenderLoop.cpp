#include "RenderSystem/RenderLoop.h"

namespace RenderSystem
{

	RenderLoop::RenderLoop()
	{
		cmdBuf_ptr = make_shared<Graphics::CommandBuffer>();
		ui_cmdBuf_ptr = make_shared<Graphics::CommandBuffer>();
		
		sync_ptr = make_shared<Graphics::Synchronization>();
		cmdQue_ptr = make_shared<Graphics::CommandQueue>(sync_ptr);
		scene_ptr = make_shared<Control::Scene>(Graphics::Vulkan::getInstance()->width, Graphics::Vulkan::getInstance()->height);
		gui_ptr = make_shared<GUI::ImguiManager>();

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

		Graphics::InitRenderPass();
		Draw::InitTextureMgr(cmdBuf_ptr);

		PhonePSO* phonePSO = new PhonePSO();
		OutlinePSO* outlinePSO = new OutlinePSO();
		
		Draw::Model* model1 = new Draw::Model(scene_ptr, "../assets/nanosuit/nanosuit.obj", "../assets/nanosuit/");
		// Draw::Model* model2 = new Draw::Model(Vulkan::getInstance(), scene_ptr, DescriptorPool::getInstance(), "../assets/plane.obj");
		phonePSO->Add(model1);
		outlinePSO->Add(model1);
		// phonePSO->Add(model2);
		pso_vecs.emplace_back(phonePSO);
		pso_vecs.emplace_back(outlinePSO);
		modelWindows.resize(10);
		gui_ptr->Init();
		gui_ptr->UpLoadFont(cmdBuf_ptr->drawCmdBuffers[0], Graphics::Vulkan::getInstance()->GetDevice().queue);
	}

	void RenderLoop::Loop()
	{
		for (size_t i = 0; i < pso_vecs.size(); ++i) {
			pso_vecs[i]->BuildPipeline();
		}
		cmdBuf_ptr->Begin();
		for (size_t i = 0; i < pso_vecs.size(); ++i) {
			pso_vecs[i]->BuildCommandBuffer(cmdBuf_ptr);
		}
		cmdBuf_ptr->End();


		while (Graphics::Vulkan::getInstance()->WindowShouldClose())
		{
			glfwPollEvents();
			if (glfwGetKey(Graphics::Vulkan::getInstance()->swapchain.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
				printf("Escape pressed. Shutting down.\n");
				break;
			}
			frameT_ptr->Record();

			scene_ptr->camera_ptr->Control_camera(Graphics::Vulkan::getInstance()->swapchain.window, frameT_ptr->Get());
			int imageIndex = cmdQue_ptr->GetCurImageIndex();

			for (size_t i = 0; i < pso_vecs.size(); ++i) {
				pso_vecs[i]->Update(imageIndex);
			}
			
			cmdQue_ptr->AddCommandBuffer(cmdBuf_ptr->drawCmdBuffers[imageIndex]);
			// renderGUI(imageIndex);
			cmdQue_ptr->Submit();
		}
	}

	void RenderLoop::renderGUI(int imageIndex)
	{
		gui_ptr->beginFrame();
		int cnt = 0;
		for (size_t i = 0; i < pso_vecs.size(); ++i) {
			auto models = pso_vecs[i]->GetModels();
			for (size_t j = 0; j < models.size(); ++j) {
				modelWindows[cnt++].SetModel(models[j]);
			}
		}
		ui_cmdBuf_ptr->Begin();
		gui_ptr->BuildCommandBuffer(ui_cmdBuf_ptr);
		ui_cmdBuf_ptr->End();
		gui_ptr->endFrame();

		cmdQue_ptr->AddCommandBuffer(ui_cmdBuf_ptr->drawCmdBuffers[imageIndex]);
	}

}