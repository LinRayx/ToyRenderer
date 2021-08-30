#ifndef RENDER_LOOP_H
#define RENDER_LOOP_H
#include "Scene.h"
#include <memory>
#include "FrameTimer.h"
#include "VulkanCore/vulkan_core_headers.h"
#include "Drawable/Texture.h"
#include "imgui/ImguiManager.h"
#include "Drawable/ModelWindow.h"

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

		void renderGUI(int imageIndex);

		shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr;
		shared_ptr<Graphics::CommandBuffer> ui_cmdBuf_ptr;
		shared_ptr<Graphics::CommandQueue> cmdQue_ptr;
		shared_ptr<Graphics::Synchronization> sync_ptr;
		shared_ptr<GUI::ImguiManager> gui_ptr;
		
		shared_ptr<Control::Scene> scene_ptr;

		std::shared_ptr<FrameTimer> frameT_ptr;

		vector<Draw::ModelWindow> modelWindows;
		vector<Draw::Model*> models;
	};
}

#endif // !RENDER_LOOP_H
