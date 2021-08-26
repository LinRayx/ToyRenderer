#ifndef SCENE_H
#define SCENE_H
#include "DynamicConstant.h"
#include "VulkanCore/vulkan_core_headers.h"

#include "Camera.h"
#include "DirectionLight.h"
#include <memory>
#include <iostream>

#include "Drawable/MaterialBase.h"

using namespace std;

namespace Control
{
	class Scene
	{
	public:
		Scene(int width, int height);
		~Scene();

		void Update(Draw::MaterialBase* material);

		shared_ptr<Camera> camera_ptr;

	private:

		DirectionLight directionLight;

		int width;
		int height;
	};
}

#endif // !SCENE_H
