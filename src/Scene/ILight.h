#ifndef I_LIGHT_H
#define I_LIGHT_H
#include <glm/glm.hpp>

namespace Control
{
	class ILight
	{
	public:
		virtual glm::vec4 GetLightColor() = 0;
		virtual glm::vec3 GetLightPosition() = 0;
		virtual void SetLightColor(glm::vec4 color) = 0;
		virtual void SetLightPosition(glm::vec3 position) = 0;
	};
}
#endif // !I_LIGHT_H
