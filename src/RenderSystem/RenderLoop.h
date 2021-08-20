#ifndef RENDER_LOOP_H
#define RENDER_LOOP_H
#include "Vulkan.h"
#include "Scene.h"
#include <memory>
#include "FrameTimer.h"
#include "RenderSystem/PhonePSO.h"
#include "VulkanCore/CommandQueue.h"

namespace RenderSystem
{
	class RenderLoop
	{

	public:
		RenderLoop();
		~RenderLoop();
		void Init();

		void Loop()
		{
			for (size_t i = 0; i < pso_vecs.size(); ++i) {
				pso_vecs[i]->BuildPipeline();
				pso_vecs[i]->BuildCommandBuffer(cmdBuf_ptr);
			}
			cmdQue_ptr->SetCommandBuffer(cmdBuf_ptr);

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
				for (size_t i = 0; i < pso_vecs.size(); ++i) {
					pso_vecs[i]->Update(imageIndex);
				}

				cmdQue_ptr->Submit();
			}
		}
	private:
		std::shared_ptr<Graphics::Vulkan> vulkan_ptr;
		std::shared_ptr<Graphics::DescriptorPool> desc_pool_ptr;
		shared_ptr<Graphics::Image> image_ptr;
		shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr;
		shared_ptr<Graphics::CommandQueue> cmdQue_ptr;
		shared_ptr<Graphics::CommandPool> cmdPool_ptr;
		shared_ptr<Graphics::Synchronization> sync_ptr;

		shared_ptr<Control::Scene> scene_ptr;

		std::shared_ptr<FrameTimer> frameT_ptr;

		vector<PipelineStateObject*> pso_vecs;
	};
}

#endif // !RENDER_LOOP_H
