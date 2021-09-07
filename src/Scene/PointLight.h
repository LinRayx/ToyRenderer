#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "Scene/Light.h"

namespace Control
{
	class PointLight : public Light
	{
	public:
		PointLight();
		float GetLightConstant();
		float GetLightLinear();
		float GetLightQuadratic();
	private:
		float constant = 1.0f;
		float linear = 0.09f;
		float quadratic = 0.032f;
	};
}

#endif // !POINT_LIGHT_H
