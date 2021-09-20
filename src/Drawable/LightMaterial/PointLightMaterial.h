#ifndef POINT_LIGHT_MATERIAL_H
#define POINT_LIGHT_MATERIAL_H

#include "Drawable/DefaultMaterial.h"
#include "Scene/PointLight.h"
#include "Utils/GloableClass.h"

namespace Draw
{
	class PointLightMaterial : public DefaultMaterial
	{
	public:
		PointLightMaterial();
		void SetPointLight(Control::PointLight* pl);
		glm::vec3 GetPosition();
		virtual void UpdateSceneData();
		
		virtual void SetTransform(glm::mat4 translate, glm::mat4 rotate, glm::mat4 scale);
		virtual bool SetUI();
	private:
		Control::PointLight* pl_ptr;
	};
}

#endif // !POINT_LIGHT_MATERIAL_H
