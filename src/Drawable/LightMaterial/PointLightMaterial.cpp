#include "Drawable/LightMaterial/PointLightMaterial.h"
namespace Draw
{
	PointLightMaterial::PointLightMaterial()
	{
		pl_ptr = nullptr;
		this->matType = MaterialType::POINTLIGHT;
	}
	void PointLightMaterial::SetPointLight(Control::PointLight* pl)
	{
		pl_ptr = pl;
		DefaultMaterial::SetColor(pl->GetLightColor());
	}
	glm::vec3 PointLightMaterial::GetPosition()
	{
		return pl_ptr->GetLightPosition();
	}
	void PointLightMaterial::SetTransform(glm::mat4 transform)
	{
		glm::vec3 pos = Gloable::ExtractTranslation(transform);
		pl_ptr->SetLightPosition(pos);
		transform = glm::scale(transform, glm::vec3(0.1f));
		MaterialBase::SetTransform(transform);
	}
	bool PointLightMaterial::SetUI()
	{
		bool dirty = DefaultMaterial::SetUI();
		pl_ptr->SetLightColor(DefaultMaterial::GetColor());
		return dirty;
	}
}