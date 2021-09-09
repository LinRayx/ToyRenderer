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

		for (auto& it : mat_fullscreen_ptrs) {
			delete it.second;
		}

		Draw::DestroyTextureMgr();
		Graphics::DestroyRenderPass();
	}

	void RenderLoop::Init()
	{	
		Draw::Model* model1 = new Draw::Model("../assets/plane.gltf", "../assets/");
		glm::mat4 I = glm::mat4(1.f);
		auto rot = I;
		// auto rot = glm::rotate(I, glm::radians(90.f), glm::vec3(1, 0, 0));
		Draw::Model* model3 = new Draw::Model("../assets/luxball.gltf", "../assets/", I, I);

		
		auto tran = glm::translate(I, glm::vec3(0, 5, -5));
		rot = glm::rotate(I, glm::radians(90.f), glm::vec3(1, 0, 0));
		
		Draw::Model* model11 = new Draw::Model("../assets/plane.gltf", "../assets/", tran, rot);

		tran = glm::translate(I, glm::vec3(-5, 5, 0));
		rot = glm::rotate(I, glm::radians(-90.f), glm::vec3(0, 0, 1));
		
		
		Draw::Model* model111 = new Draw::Model("../assets/plane.gltf", "../assets/", tran, rot);

		model111->AddMaterial(Draw::MaterialType::GBuffer);
		model111->AddMaterial(Draw::MaterialType::OMNISHADOW);

		model11->AddMaterial(Draw::MaterialType::GBuffer);
		model11->AddMaterial(Draw::MaterialType::OMNISHADOW);

		model1->AddMaterial(Draw::MaterialType::GBuffer);
		model3->AddMaterial(Draw::MaterialType::GBuffer);
		model1->AddMaterial(Draw::MaterialType::OMNISHADOW);
		model3->AddMaterial(Draw::MaterialType::OMNISHADOW);

		Draw::Model* model2 = new Draw::Model( "../assets/cube.gltf", "../assets/");
		model2->AddMaterial(Draw::MaterialType::Skybox);

		Draw::Model* lightModel = new Draw::Model("../assets/cube.gltf", "../assets/");
		Draw::PointLightMaterial* plMaterial = new Draw::PointLightMaterial;
		plMaterial->SetPointLight(&Control::Scene::getInstance()->pointLights[0]);
		lightModel->AddMaterial(plMaterial);

		models.emplace_back(std::move(model2));
		models.emplace_back(std::move(model1));
		models.emplace_back(std::move(model3));
		models.emplace_back(std::move(lightModel));
		models.emplace_back(std::move(model11));
		models.emplace_back(std::move(model111));

		modelWindows[0].SetModel(models[1]);
		modelWindows[1].SetModel(models[2]);
		modelWindows[2].SetModel(models[3]);
		for (auto& model : models) {
			model->Compile();
		}
		Draw::SSAOgenerateMaterial* ssao_ptr = new Draw::SSAOgenerateMaterial;
		ssao_ptr->Compile();
		mat_fullscreen_ptrs[Draw::MaterialType::FS_SSAO] = ssao_ptr;

		Draw::BlurMaterial* blur_ptr = new Draw::BlurMaterial;
		blur_ptr->AddBlurMap("ssaoMap");
		blur_ptr->Compile();
		mat_fullscreen_ptrs[Draw::MaterialType::FS_BLUR] = blur_ptr;

		Draw::PbrDeferredMaterial* pbr_deferred = new Draw::PbrDeferredMaterial;
		pbr_deferred->Compile();
		mat_fullscreen_ptrs[Draw::MaterialType::PBR_Deferred] = pbr_deferred;

		buildCmd();
	}

	void RenderLoop::PreSolve()
	{
		Gloable::SSAO::InitSSAOKernel();
		Draw::InitTextureMgr(cmdBuf_ptr);
		Draw::CreateRenderPass();
		Bind::LoadShaders();
		UIInit();

		Draw::BrdfMaterial* brdfLUT = new Draw::BrdfMaterial();
		brdfLUT->Compile();
		brdfLUT->Execute(cmdBuf_ptr);
		delete brdfLUT;

		Draw::Model* model2 = new Draw::Model("../assets/cube.gltf", "../assets/");

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

			Control::Scene::getInstance()->camera_ptr->Control_camera(Graphics::Vulkan::getInstance()->swapchain.window, frameT_ptr->Get());
			int imageIndex = cmdQue_ptr->GetCurImageIndex();

			for (auto& mat : mat_fullscreen_ptrs) {
				mat.second->UpdateSceneData();
				mat.second->Update(imageIndex);
			}

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
		ImGui::Begin("Hello, world!");
		auto viewPos = Control::Scene::getInstance()->camera_ptr->GetViewPos();
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("ViewPos:  %.3f , %.3f, %.3f . Yaw: %.2f, Pitch: %.2f,", viewPos.x, viewPos.y, viewPos.z, 
			Control::Scene::getInstance()->camera_ptr->GetYaw(),
			Control::Scene::getInstance()->camera_ptr->GetPitch());
		gui_ptr->updated = ImGui::Checkbox("SSAO Open", &Control::Scene::getInstance()->SSAO);
		ImGui::End();
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

		for (int cmdIndex = 0; cmdIndex < 2; ++cmdIndex) {
			for (int i = 0; i < 6; ++i) {
				cmdBuf_ptr->ShadowBegin(Graphics::RenderPassType::ONMISHADOW, cmdIndex);
				for (auto& model : models) {
					model->BuildCommandBuffer(Draw::MaterialType::OMNISHADOW, cmdBuf_ptr,cmdIndex, i);
				}
				cmdBuf_ptr->ShadowEnd(cmdIndex);
				Graphics::Image::getInstance()->CopyFrameBufferToImage2(cmdBuf_ptr->drawCmdBuffers[cmdIndex],
					Draw::textureManager->nameToTex["omni_depth_map"].textureImage,
					Draw::textureManager->nameToTex["omni_color_attachment"].textureImage,
					1024,
					i);
			}
		}


		cmdBuf_ptr->DeferredBegin();
		for (auto& model : models) {
			model->BuildCommandBuffer(Draw::MaterialType::GBuffer, cmdBuf_ptr);
		}
		cmdBuf_ptr->DeferredEnd();

		// full screen pass, output ssaoMap
		mat_fullscreen_ptrs[Draw::MaterialType::FS_SSAO]->BuildCommandBuffer(cmdBuf_ptr);
		mat_fullscreen_ptrs[Draw::MaterialType::FS_BLUR]->BuildCommandBuffer(cmdBuf_ptr);

		cmdBuf_ptr->DefaultBegin();

		mat_fullscreen_ptrs[Draw::MaterialType::PBR_Deferred]->BuildCommandBuffer(cmdBuf_ptr);

		// Draw Light
		for (auto& model : models) {
			model->BuildCommandBuffer(Draw::MaterialType::POINTLIGHT, cmdBuf_ptr);
		}
		for (auto& model : models) {
		 	 // model->BuildCommandBuffer(Draw::MaterialType::Skybox, cmdBuf_ptr);
		}
		renderGUI();
		cmdBuf_ptr->DefaultEnd();

		cmdBuf_ptr->End();
	}

}