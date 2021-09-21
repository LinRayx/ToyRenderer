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
		// defaultScene();
		csmScene();
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

		Draw::CompositionMaterial* comp_ptr = new Draw::CompositionMaterial;
		comp_ptr->Compile();
		mat_fullscreen_ptrs[Draw::MaterialType::FS_COMPOSITION] = comp_ptr;

		buildCmd();
	}

	void RenderLoop::PreSolve()
	{
		Gloable::SSAO::InitSSAOKernel();
		Gloable::ReadpreComputeFile();
		Draw::InitTextureMgr(cmdBuf_ptr);
		Draw::CreateRenderPass();
		Bind::LoadShaders();
		Bind::LoadShaderPaths();
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

			for (auto& model : models) {
				model->Update(imageIndex);
			}

			for (auto& mat : mat_fullscreen_ptrs) {
				mat.second->UpdateSceneData();
				mat.second->Update(imageIndex);
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

		ImGui::SliderFloat("CascadeSplitLambda", &Draw::Cascades::GetCascadeSplitLambda(), 0, 1.f);
		ImGui::SliderInt("CascadeIndex", &Gloable::CASCADEINDEX, 0, Draw::SHADOWMAP_COUNT-1);
		// gui_ptr->updated = ImGui::Checkbox("SSAO Open", &Control::Scene::getInstance()->SSAO);
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
		
		for (int cmdIndex = 0; cmdIndex < cmdBuf_ptr->drawCmdBuffers.size(); ++cmdIndex) {
			for (int i = 0; i < Draw::SHADOWMAP_COUNT; ++i) {
				cmdBuf_ptr->ShadowBegin(Graphics::RenderPassType::CASCADE_SHADOW, cmdIndex, i);
				for (auto& model : models) {
					model->BuildCommandBuffer(Draw::MaterialType::CASCADESHADOW, cmdBuf_ptr, cmdIndex, i);
				}
				cmdBuf_ptr->ShadowEnd(cmdIndex);
			}
		}

		// csm_ptr->BuildCommandBuffer(cmdBuf_ptr);
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
		mat_fullscreen_ptrs[Draw::MaterialType::PBR_Deferred]->BuildCommandBuffer(cmdBuf_ptr);

		cmdBuf_ptr->DefaultBegin();

		mat_fullscreen_ptrs[Draw::MaterialType::FS_COMPOSITION]->BuildCommandBuffer(cmdBuf_ptr);

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

	void RenderLoop::defaultScene()
	{
		Draw::Model* planeBottom = new Draw::Model("../assets/plane.gltf", "../assets/");

		glm::mat4 I = glm::mat4(1.f);
		auto rot = I;
		rot = glm::rotate(I, glm::radians(-90.f), glm::vec3(1, 0, 0));
		rot = glm::rotate(rot, glm::radians(180.f), glm::vec3(0, 0, 1));
		Draw::Model* mary = new Draw::Model("../assets/mary.gltf", "../assets/", I, rot);

		auto tran = glm::translate(I, glm::vec3(0, 5, -5));
		rot = glm::rotate(I, glm::radians(90.f), glm::vec3(1, 0, 0));
		Draw::Model* planeLeft = new Draw::Model("../assets/plane.gltf", "../assets/", tran, rot);

		tran = glm::translate(I, glm::vec3(-5, 5, 0));
		rot = glm::rotate(I, glm::radians(-90.f), glm::vec3(0, 0, 1));
		Draw::Model* planeBack = new Draw::Model("../assets/plane.gltf", "../assets/", tran, rot);

		planeBack->AddMaterial(Draw::MaterialType::GBuffer);
		planeBack->AddMaterial(Draw::MaterialType::OMNISHADOW);
		planeBack->AddMaterial(Draw::MaterialType::CASCADESHADOW);

		planeLeft->AddMaterial(Draw::MaterialType::GBuffer);
		planeLeft->AddMaterial(Draw::MaterialType::OMNISHADOW);
		planeLeft->AddMaterial(Draw::MaterialType::CASCADESHADOW);

		planeBottom->AddMaterial(Draw::MaterialType::GBuffer);
		planeBottom->AddMaterial(Draw::MaterialType::OMNISHADOW);
		planeBottom->AddMaterial(Draw::MaterialType::CASCADESHADOW);

		mary->AddMaterial(Draw::MaterialType::GBuffer);
		mary->AddMaterial(Draw::MaterialType::OMNISHADOW);
		mary->AddMaterial(Draw::MaterialType::CASCADESHADOW);

		Draw::Model* skybox = new Draw::Model("../assets/cube.gltf", "../assets/");
		skybox->AddMaterial(Draw::MaterialType::Skybox);

		tran = glm::translate(I, glm::vec3(0, 3, 5));
		rot = I;
		auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
		Draw::Model* lightModel = new Draw::Model("../assets/cube.gltf", "../assets/", tran, rot, scale);
		Draw::PointLightMaterial* plMaterial = new Draw::PointLightMaterial;
		plMaterial->SetPointLight(&Control::Scene::getInstance()->pointLights[0]);
		lightModel->AddMaterial(plMaterial);

		models.emplace_back(std::move(skybox));
		models.emplace_back(std::move(planeBottom));
		models.emplace_back(std::move(mary));
		models.emplace_back(std::move(lightModel));
		models.emplace_back(std::move(planeLeft));
		models.emplace_back(std::move(planeBack));

		modelWindows[0].SetModel(models[1]);
		modelWindows[1].SetModel(models[2]);
		modelWindows[2].SetModel(models[3]);
	}

	void RenderLoop::csmScene()
	{
		glm::mat4 I = glm::mat4(1.f);
		auto tran = I;
		auto rot = I;
		auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(10.f));
		Draw::Model* planeBottom = new Draw::Model("../assets/plane.gltf", "../assets/", tran, rot, scale);
		models.emplace_back(std::move(planeBottom));
		planeBottom->AddMaterial(Draw::MaterialType::GBuffer);
		planeBottom->AddMaterial(Draw::MaterialType::OMNISHADOW);
		planeBottom->AddMaterial(Draw::MaterialType::CASCADESHADOW);
		auto startPoint = I;
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f));
		for (int i = -4; i < 4; ++i) {
			for (int j = -4; j < 4; ++j) {
				tran = glm::translate(startPoint, glm::vec3(j * 10, 1, i * 10));
				
				Draw::Model* cube = new Draw::Model("../assets/cube.gltf", "../assets/", tran, rot, scale);
				cube->AddMaterial(Draw::MaterialType::GBuffer);
				cube->AddMaterial(Draw::MaterialType::OMNISHADOW);
				cube->AddMaterial(Draw::MaterialType::CASCADESHADOW);
				models.emplace_back(std::move(cube));
			}
		}

		//tran = glm::translate(I, glm::vec3(0, 3, 5));
		//rot = I;
		//scale = glm::scale(glm::mat4(1.0), glm::vec3(0.1));
		//Draw::Model* lightModel = new Draw::Model("../assets/cube.gltf", "../assets/", tran, rot, scale);
		//Draw::PointLightMaterial* plMaterial = new Draw::PointLightMaterial;
		//plMaterial->SetPointLight(&Control::Scene::getInstance()->pointLights[0]);
		//lightModel->AddMaterial(plMaterial);
		//models.emplace_back(std::move(lightModel));
	}

}