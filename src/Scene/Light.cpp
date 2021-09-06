#include "Scene/Light.h"

namespace Control
{
	Light::Light()
	{
	}
	glm::vec4 Light::GetLightColor()
	{
		return color;
	}
	glm::vec3 Light::GetLightPosition()
	{
		return position;
	}
	void Light::SetLightColor(glm::vec4 color)
	{
		this->color = color;
	}
	void Light::SetLightPosition(glm::vec3 position)
	{
		this->position = position;
	}

}