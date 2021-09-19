#ifndef RENDER_LOOP_H
#define RENDER_LOOP_H
#include "Scene.h"
#include <memory>
#include "FrameTimer.h"
#include "VulkanCore/vulkan_core_headers.h"
#include "Drawable/Texture.h"
#include "imgui/ImguiManager.h"
#include "Drawable/ModelWindow.h"
#include "Drawable/MaterialHeader.h"
#include "Drawable/RenderPassFactory.h"
#include "Scene/PointLight.h"

namespace RenderSystem
{
	class RenderLoop
	{

	public:
		RenderLoop();
		~RenderLoop();
		void Init();
		void PreSolve();
		void Loop();

		void Run();
	private:
		void UIInit();
		void renderGUI();
		void updateUI();
		void buildCmd();

		shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr;
		shared_ptr<Graphics::CommandBuffer> ui_cmdBuf_ptr;
		shared_ptr<Graphics::CommandQueue> cmdQue_ptr;
		shared_ptr<Graphics::Synchronization> sync_ptr;
		shared_ptr<GUI::ImguiManager> gui_ptr;

		shared_ptr<Draw::CascadeShadowMaterial> csm_ptr;
		
		std::shared_ptr<FrameTimer> frameT_ptr;

		vector<Draw::ModelWindow> modelWindows;
		vector<Draw::Model*> models;
		map<Draw::MaterialType, Draw::MaterialBaseParent*> mat_fullscreen_ptrs;

		int width = 1280;
		int height = 760;
	};
}

#endif // !RENDER_LOOP_H
