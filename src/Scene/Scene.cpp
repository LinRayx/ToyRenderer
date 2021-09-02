#include "Scene.h"

namespace Control
{

	Scene::Scene(int width, int height)
		 :width(width), height(height)
	{
		camera_ptr = make_shared<Camera>(width, height);
	}

	Scene::~Scene()
	{
	}

	void Scene::Update(Draw::MaterialBase* material)
	{
		if (material->GetMaterailType() == Draw::MaterialType::Skybox) {
			material->SetValue("ViewAndProj", "viewMat", glm::mat4(glm::mat3(camera_ptr->GetViewMatrix())));
		}
		else {
			material->SetValue("ViewAndProj", "viewMat", camera_ptr->GetViewMatrix());
		}

		if (material->GetMaterailType() == Draw::MaterialType::GBuffer) {
			material->SetValue("ViewAndProj", "nearPlane", camera_ptr->GetNearPlane());
			material->SetValue("ViewAndProj", "nearPlane", camera_ptr->GetFarPlane());
		}

		material->SetValue("ViewAndProj", "projMat", camera_ptr->GetProjectMatrix());
		material->SetValue("Light", "viewPos", camera_ptr->GetViewPos());
		material->SetValue("Light", "direLightDir", directionLight.direciton);
		material->SetValue("Light", "direLightColor", directionLight.color);
	}

}