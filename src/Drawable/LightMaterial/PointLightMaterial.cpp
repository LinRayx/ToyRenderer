#include "Drawable/LightMaterial/PointLightMaterial.h"
#include <glm/gtx/string_cast.hpp>
namespace Draw
{
	PointLightMaterial::PointLightMaterial() : DefaultMaterial()
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
	void PointLightMaterial::UpdateSceneData()
	{
		MaterialBase::UpdateSceneData();
	}
	void PointLightMaterial::SetTransform(glm::mat4 translate, glm::mat4 rotate, glm::mat4 scale)
	{
		glm::mat4 transform = translate * rotate * scale;
		glm::vec3 pos = Gloable::ExtractTranslation(translate);
		pl_ptr->SetLightPosition(pos);
		SetValue("Model", "modelTrans", transform);
	}
	bool PointLightMaterial::SetUI()
	{
		bool dirty = DefaultMaterial::SetUI();
		pl_ptr->SetLightColor(DefaultMaterial::GetColor());
		return dirty;
	}
}