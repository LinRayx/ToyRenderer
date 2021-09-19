#include "DirectionLight.h"

Control::DirectionLight::DirectionLight() : Light()
{
}

glm::vec3 Control::DirectionLight::GetDirection()
{
	return direction;
}

void Control::DirectionLight::SetDirection(glm::vec3 dir)
{
	this->direction = dir;
}
