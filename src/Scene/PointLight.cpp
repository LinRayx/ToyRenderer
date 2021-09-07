#include "Scene/PointLight.h"

namespace Control
{
	PointLight::PointLight() : Light()
	{
	}

	float PointLight::GetLightConstant()
	{
		return constant;
	}

	float PointLight::GetLightLinear()
	{
		return linear;
	}

	float PointLight::GetLightQuadratic()
	{
		return quadratic;
	}

}