#ifndef DIRECTION_LIGHT_H
#define DIRECTION_LIGHT_H
#include "Scene/Light.h"

namespace Control
{
	class DirectionLight : public Light
	{
		friend class Scene;
	public:
		DirectionLight();
		glm::vec3 GetDirection();
		void SetDirection(glm::vec3 dir);
	private:
		glm::vec3 direction;
	};
}

#endif // !DIRECTION_LIGHT_H
