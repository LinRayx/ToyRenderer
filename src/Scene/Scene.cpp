#include "Scene.h"

namespace Control
{

	Scene::Scene(shared_ptr<Graphics::Vulkan> vulkan_ptr, int width, int height)
		: vulkan_ptr(vulkan_ptr),  width(width), height(height)
	{
		camera_ptr = make_shared<Camera>(width, height);
	}

	Scene::~Scene()
	{
	}

	void Scene::InitSceneData(Draw::MaterialBase* material)
	{
	}

	void Scene::Update(Draw::MaterialBase* material)
	{
		material->SetValue("ViewAndProj", "viewMat", camera_ptr->GetViewMatrix());
		material->SetValue("ViewAndProj", "projMat", camera_ptr->GetProjectMatrix());
		material->SetValue("Light", "viewPos", camera_ptr->GetViewPos());
		material->SetValue("Light", "direLightDir", directionLight.direciton);
		material->SetValue("Light", "direLightColor", directionLight.color);
	}

}