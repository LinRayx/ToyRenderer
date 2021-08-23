#ifndef RENDER_LOOP_H
#define RENDER_LOOP_H
#include "Vulkan.h"
#include "Scene.h"
#include <memory>
#include "FrameTimer.h"
#include "RenderSystem/PhonePSO.h"
#include "VulkanCore/CommandQueue.h"
#include "Drawable/Texture.h"
#include "imgui/ImguiManager.h"

namespace RenderSystem
{
	class RenderLoop
	{

	public:
		RenderLoop();
		~RenderLoop();
		void Init();

		void Loop();
	private:
		std::shared_ptr<Graphics::Vulkan> vulkan_ptr;
		std::shared_ptr<Graphics::DescriptorPool> desc_pool_ptr;
		shared_ptr<Graphics::Image> image_ptr;
		shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr;
		shared_ptr<Graphics::CommandQueue> cmdQue_ptr;
		shared_ptr<Graphics::CommandPool> cmdPool_ptr;
		shared_ptr<Graphics::Synchronization> sync_ptr;
		shared_ptr<GUI::ImguiManager> gui_ptr;
		
		shared_ptr<Draw::Texture> texture_ptr;

		shared_ptr<Control::Scene> scene_ptr;

		std::shared_ptr<FrameTimer> frameT_ptr;

		vector<PipelineStateObject*> pso_vecs;
	};
}

#endif // !RENDER_LOOP_H
