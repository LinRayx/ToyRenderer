#ifndef LIGHT_H
#define LIGHT_H
#include "Scene/ILight.h"
#include <string>


namespace Control
{
	class Scene;
	class Light : public ILight
	{
		friend class Scene;
	public:
		Light();
		virtual glm::vec4 GetLightColor() override;
		virtual glm::vec3 GetLightPosition() override;
		virtual void SetLightColor(glm::vec4 color) override;
		virtual void SetLightPosition(glm::vec3 position) override;

	protected:
		
		glm::vec4 color = glm::vec4(1, 1, 1, 1);
		glm::vec3 position = glm::vec3(0, 3, 5);
	};
}

#endif // !LIGHT_H
