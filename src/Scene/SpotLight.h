#ifndef SPOT_LIGHT_H
#define SPOT_LIGHT_H

#include "Scene/Light.h"

namespace Control
{
	class SpotLight : public Light
	{
	public:
		SpotLight();
	private:
		glm::vec3  direction;
		float cutOff;
	};
}

#endif // !SPOT_LIGHT_H
