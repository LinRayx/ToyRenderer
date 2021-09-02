#include "RenderSystem/RenderLoop.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

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

		modelWindows.resize(10);

	}

	RenderLoop::~RenderLoop()
	{
		gui_ptr->freeResources();
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
		model1->AddMaterial(Draw::MaterialType::GBuffer);
		model1->AddMaterial(Draw::MaterialType::Outline);

		Draw::Model* model2 = new Draw::Model(scene_ptr, "../assets/cube.obj", "../assets/cube.obj");
		model2->AddMaterial(Draw::MaterialType::Skybox);

		models.emplace_back(std::move(model2));
		models.emplace_back(std::move(model1));
		modelWindows[0].SetModel(models[1]);

		for (auto& model : models) {
			model->Compile();
		}

		buildCmd();
	}

	void RenderLoop::PreSolve()
	{
		Draw::InitTextureMgr(cmdBuf_ptr);
		Graphics::InitRenderPass();
		Bind::LoadShaders();
		UIInit();

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
			cmdQue_ptr->Submit();

			updateUI();
		}
	}

	void RenderLoop::Run()
	{
		PreSolve();
		Init();
		Loop();
	}

	void RenderLoop::UIInit()
	{
		gui_ptr->loadShaders();
		gui_ptr->prepareResources();
		gui_ptr->preparePipeline(VK_NULL_HANDLE, Graphics::nameToRenderPass[Graphics::RenderPassType::Default]->renderPass);
	}

	void RenderLoop::renderGUI()
	{
		gui_ptr->draw(cmdBuf_ptr);
	}

	void RenderLoop::updateUI()
	{
		ImGuiIO& io = ImGui::GetIO();

		io.DisplaySize = ImVec2((float)width, (float)height);
		io.DeltaTime = frameT_ptr->Get();
		io.MousePos = ImVec2(Control::mousePos.x, Control::mousePos.y);
		io.MouseDown[0] = Control::mouseButtons.left;
		io.MouseDown[1] = Control::mouseButtons.right;

		ImGui::NewFrame();
		for (auto& window : modelWindows) {
			gui_ptr->updated = window.DrawUI();
		}
		ImGui::Render();

		if (gui_ptr->update() || gui_ptr->updated)
		{
			buildCmd();
			gui_ptr->updated = false;
		}
	}

	void RenderLoop::buildCmd()
	{
		cmdBuf_ptr->Begin();

		cmdBuf_ptr->DeferredBegin();
		for (auto& model : models) {
			model->BuildCommandBuffer(Draw::MaterialType::GBuffer, cmdBuf_ptr);
		}
		cmdBuf_ptr->DeferredEnd();

		cmdBuf_ptr->DefaultBegin();
		for (auto& model : models) {
			model->BuildCommandBuffer(Draw::MaterialType::Skybox, cmdBuf_ptr);
		}

		for (auto& model : models) {
			model->BuildCommandBuffer(Draw::MaterialType::PBR, cmdBuf_ptr);
		}

		for (auto& model : models) {
			model->BuildCommandBuffer(Draw::MaterialType::Outline, cmdBuf_ptr);
		}
		renderGUI();
		cmdBuf_ptr->DefaultEnd();

		cmdBuf_ptr->End();
	}

}