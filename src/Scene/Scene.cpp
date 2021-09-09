#include "Scene.h"

namespace Control
{

	Scene::Scene(int width, int height)
		 : width(width), height(height)
	{
		camera_ptr = make_shared<Camera>(width, height);
		PointLight pl;
		// pl.SetLightPosition(glm::vec3(3, -3, -3));
		pointLights.emplace_back(std::move(pl));
	}

	Scene::~Scene()
	{
	}

	Scene* Scene::instance = NULL;
}