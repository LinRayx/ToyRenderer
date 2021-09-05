#include "Scene.h"

namespace Control
{

	Scene::Scene(int width, int height)
		 : width(width), height(height)
	{
		camera_ptr = make_shared<Camera>(width, height);
		//camera_ptr->setPosition(glm::vec3(0.0f, 0.0f, -10.25f));
		// camera_ptr->setRotation(glm::vec3(-0.75f, 12.5f, 0.0f));

		//camera_ptr->setPerspective(45.0f, (float)width / (float)height, 0.1f, 256.0f);
	}

	Scene::~Scene()
	{
	}

	Scene* Scene::instance = NULL;
}