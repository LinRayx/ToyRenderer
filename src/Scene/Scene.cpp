#include "Scene.h"

namespace Control
{

	Scene::Scene(int width, int height)
		 : width(width), height(height)
	{
		camera_ptr = make_shared<Camera>(width, height);
	}

	Scene::~Scene()
	{
	}

	Scene* Scene::instance = NULL;
}