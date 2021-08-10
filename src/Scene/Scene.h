#ifndef SCENE_H
#define SCENE_H
#include "DynamicConstant.h"
#include "Buffer.h"
#include "DescriptorSet.h"
#include "Camera.h"
#include "DirectionLight.h"
#include <memory>
#include <iostream>
using namespace std;

namespace Control
{
	class Scene
	{
	public:
		Scene(shared_ptr<Graphics::Vulkan> vulkan_ptr, shared_ptr<Graphics::DescriptorSetCore> desc_ptr, int width, int height);
		~Scene();

		void InitSceneData();
		void Update();

		shared_ptr<Camera> camera_ptr;
		shared_ptr<Graphics::DescriptorSetCore> desc_ptr;
	private:
		shared_ptr<Graphics::Vulkan> vulkan_ptr;
		
		DirectionLight directionLight;

		int width;
		int height;

		std::map<std::string, shared_ptr<Graphics::Buffer>> buffer_ptrs;
		std::map<std::string, shared_ptr<Dcb::Buffer>> bufs;
	};
}

#endif // !SCENE_H
