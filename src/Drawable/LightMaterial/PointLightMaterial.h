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
		virtual void SetTransform(glm::mat4 transform);
		virtual bool SetUI();
	private:
		Control::PointLight* pl_ptr;
	};
}

#endif // !POINT_LIGHT_MATERIAL_H
