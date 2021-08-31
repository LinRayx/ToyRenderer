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
		// gui_ptr = make_shared<GUI::ImguiManager>();

		frameT_ptr = std::make_shared<FrameTimer>();
	}

	RenderLoop::~RenderLoop()
	{
		for (size_t i = 0; i < models.size(); ++i) {
			if (models[i] != nullptr) {
				delete models[i];
			}
		}
		Draw::DestroyTextureMgr();
		Graphics::DestroyRenderPass();
	}

	void RenderLoop::Init()
	{	
		Draw::Model* model1 = new Draw::Model(scene_ptr, "../assets/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX", "../assets/Cerberus_by_Andrew_Maximov/");
		model1->AddMaterial(Draw::MaterialType::PBR);
		model1->AddMaterial(Draw::MaterialType::Outline);

		Draw::Model* model2 = new Draw::Model(scene_ptr, "../assets/cube.obj", "../assets/cube.obj");
		model2->AddMaterial(Draw::MaterialType::Skybox);

		models.emplace_back(model2);
		models.emplace_back(model1);

		cmdBuf_ptr->Begin();
		for (auto& model : models) {
			model->Compile();
		}

		for (auto& model : models) {
			model->BuildCommandBuffer(Draw::MaterialType::Skybox, cmdBuf_ptr);
		}

		for (auto& model : models) {
			model->BuildCommandBuffer(Draw::MaterialType::PBR, cmdBuf_ptr);
		}

		for (auto& model : models) {
			model->BuildCommandBuffer(Draw::MaterialType::Outline, cmdBuf_ptr);
		}

		cmdBuf_ptr->End();

		modelWindows.resize(10);
		// gui_ptr->Init();
		// gui_ptr->UpLoadFont(cmdBuf_ptr->drawCmdBuffers[0], Graphics::Vulkan::getInstance()->GetDevice().queue);
	}

	void RenderLoop::PreSolve()
	{
		Draw::InitTextureMgr(cmdBuf_ptr);
		Graphics::InitRenderPass();
		Bind::LoadShaders();

		Draw::BrdfMaterial* brdfLUT = new Draw::BrdfMaterial();
		brdfLUT->Compile();
		brdfLUT->Execute(cmdBuf_ptr);
		delete brdfLUT;

		Draw::Model* model2 = new Draw::Model(scene_ptr, "../assets/cube.obj", "../assets/cube.obj");

		Draw::IrradianceMaterial* irradiance = new Draw::IrradianceMaterial();
		irradiance->BindMeshData(model2->objects[0].mesh.vertex_buffer, model2->objects[0].mesh.index_buffer);
		irradiance->Compile();
		irradiance->Execute(cmdBuf_ptr);
		delete irradiance;

		Draw::PrefilterMaterial* prefilter = new Draw::PrefilterMaterial();
		prefilter->BindMeshData(model2->objects[0].mesh.vertex_buffer, model2->objects[0].mesh.index_buffer);
		prefilter->Compile();
		prefilter->Execute(cmdBuf_ptr);
		delete prefilter;

		delete model2;
	}

	void RenderLoop::Loop()
	{
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

			for (auto& model : models) {
				model->Update(imageIndex);
			}
			
			cmdQue_ptr->AddCommandBuffer(cmdBuf_ptr->drawCmdBuffers[imageIndex]);
			// renderGUI(imageIndex);
			cmdQue_ptr->Submit();
		}
	}

	void RenderLoop::Run()
	{
		PreSolve();
		Init();
		Loop();
	}

	void RenderLoop::renderGUI(int imageIndex)
	{
		gui_ptr->beginFrame();
		int cnt = 0;

		ui_cmdBuf_ptr->Begin();
		gui_ptr->BuildCommandBuffer(ui_cmdBuf_ptr);
		ui_cmdBuf_ptr->End();
		gui_ptr->endFrame();

		cmdQue_ptr->AddCommandBuffer(ui_cmdBuf_ptr->drawCmdBuffers[imageIndex]);
	}

}