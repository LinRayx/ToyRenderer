//
//#include <vld.h>
#include "RenderSystem/RenderLoop.h"

using namespace std;

int WIDTH = 800;
int HEIGHT = 600;

// Graphics::Vulkan::Deletor Graphics::Vulkan::deletor;
Graphics::DescriptorPool::Deletor Graphics::DescriptorPool::deletor;
Graphics::CommandPool::Deletor Graphics::CommandPool::deletor;
Graphics::Image::Deletor Graphics::Image::deletor;
Control::Scene::Deletor Control::Scene::deletor;

int main()
{
	RenderSystem::RenderLoop loop;
	loop.Run();
	auto* op = new int;
	return 0;
}