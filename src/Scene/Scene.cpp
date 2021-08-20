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

	void Scene::InitSceneData(Draw::Material* material)
	{
		Dcb::RawLayout layout;
		layout.Add<Dcb::Matrix>("viewMat");
		layout.Add<Dcb::Matrix>("projMat");

		Dcb::RawLayout layout2;
		layout2.Add<Dcb::Float3>("viewPos");
		layout2.Add<Dcb::Float3>("direLightDir");
		layout2.Add<Dcb::Float3>("direLightColor");

		material->AddLayout("ViewAndProj", std::move(layout), Graphics::LayoutType::SCENE, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::VERTEX);
		material->AddLayout("Light", std::move(layout2), Graphics::LayoutType::SCENE, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::FRAGMENT);

		material->Update("Light", "direLightDir", directionLight.direciton);
		material->Update("Light", "direLightColor", directionLight.color);
	}

	void Scene::Update(Draw::Material* material)
	{
		material->Update("ViewAndProj", "viewMat", camera_ptr->GetViewMatrix());
		material->Update("ViewAndProj", "projMat", camera_ptr->GetProjectMatrix());
		material->Update("Light", "viewPos", camera_ptr->GetViewPos());
	}

}