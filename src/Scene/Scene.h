#ifndef SCENE_H
#define SCENE_H
#include "VulkanCore/vulkan_core_headers.h"
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

		~Scene();
		shared_ptr<Camera> camera_ptr;
		static Scene* getInstance() {
			if (instance == NULL) {
				instance = new Scene(Graphics::Vulkan::getInstance()->GetWidth(), Graphics::Vulkan::getInstance()->GetHeight());
			}
			return instance;
		}
		DirectionLight directionLight;
		bool SSAO = true;
	private:
		Scene(int width, int height);
		class Deletor {
		public:
			~Deletor() {
				if (Scene::instance != NULL)
					delete Scene::instance;
			}
		};
		static Deletor deletor;

		int width;
		int height;
		static Scene* instance;
	};
}

#endif // !SCENE_H
