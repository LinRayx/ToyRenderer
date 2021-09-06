#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "Scene/Light.h"

namespace Control
{
	class PointLight : public Light
	{
	public:
		PointLight();

	private:
		float constant;
		float linear;
		float quadratic;
	};
}

#endif // !POINT_LIGHT_H
