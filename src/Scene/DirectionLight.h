#ifndef DIRECTION_LIGHT_H
#define DIRECTION_LIGHT_H
#include <glm/glm.hpp>

namespace Control
{
	class DirectionLight
	{
		friend class Scene;
	public:
		DirectionLight();
	private:
		glm::vec3 direciton;
		glm::vec3 color;
	};
}

#endif // !DIRECTION_LIGHT_H
