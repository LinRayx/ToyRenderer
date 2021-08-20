#ifndef SCENE_H
#define SCENE_H
#include "DynamicConstant.h"
#include "Buffer.h"
#include "DescriptorSet.h"
#include "Camera.h"
#include "DirectionLight.h"
#include <memory>
#include <iostream>

#include "Drawable/Material.h"

using namespace std;

namespace Control
{
	class Scene
	{
	public:
		Scene(shared_ptr<Graphics::Vulkan> vulkan_ptr, int width, int height);
		~Scene();

		void InitSceneData(Draw::Material* material);
		void Update(Draw::Material* material);

		shared_ptr<Camera> camera_ptr;

	private:
		shared_ptr<Graphics::Vulkan> vulkan_ptr;
		
		DirectionLight directionLight;

		int width;
		int height;
	};
}

#endif // !SCENE_H
