#include "Scene.h"

namespace Control
{

	Scene::Scene(shared_ptr<Graphics::Vulkan> vulkan_ptr, shared_ptr<Graphics::DescriptorSetCore> desc_ptr, int width, int height)
		: vulkan_ptr(vulkan_ptr), desc_ptr(desc_ptr), width(width), height(height)
	{
		camera_ptr = make_shared<Camera>(width, height);
	}

	Scene::~Scene()
	{
	}

	void Scene::InitSceneData()
	{
		Dcb::RawLayout layout;
		layout.Add<Dcb::Matrix>("viewMat");
		layout.Add<Dcb::Matrix>("projMat");

		Dcb::RawLayout layout2;
		layout2.Add<Dcb::Float3>("viewPos");
		layout2.Add<Dcb::Float3>("direLightDir");
		layout2.Add<Dcb::Float3>("direLightColor");

		bufs["ViewAndProj"] = make_shared<Dcb::Buffer>(std::move(layout));
		buffer_ptrs["ViewAndProj"] = make_shared<Graphics::Buffer>(vulkan_ptr, Graphics::BufferUsage::UNIFORM, bufs["ViewAndProj"]->GetSizeInBytes());
		desc_ptr->Add(Graphics::LayoutType::SCENE, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::VERTEX, buffer_ptrs["ViewAndProj"]);

		bufs["Light"] = make_shared<Dcb::Buffer>(std::move(layout2));
		buffer_ptrs["Light"] = make_shared<Graphics::Buffer>(vulkan_ptr, Graphics::BufferUsage::UNIFORM, bufs["Light"]->GetSizeInBytes());
		desc_ptr->Add(Graphics::LayoutType::SCENE, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::FRAGMENT, buffer_ptrs["Light"]);



		(*bufs["Light"])["direLightDir"] = directionLight.direciton;
		(*bufs["Light"])["direLightColor"] = directionLight.color;

	}

	void Scene::Update()
	{
		(*bufs["ViewAndProj"])["viewMat"] = camera_ptr->GetViewMatrix();
		(*bufs["ViewAndProj"])["projMat"] = camera_ptr->GetProjectMatrix();
		(*bufs["Light"])["viewPos"] = camera_ptr->GetViewPos();

		for (auto it : buffer_ptrs)
		{
			for (int i = 0; i < 2; ++i)
			{
				it.second->UpdateData(i, bufs[it.first]->GetSizeInBytes(), bufs[it.first]->GetData());
			}
		}
	}

}