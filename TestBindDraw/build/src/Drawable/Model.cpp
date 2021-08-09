#include "Model.h"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Draw::Model::getModelMatrix()
{
	glm::mat4 mat = glm::mat4(1.0);
	mat = glm::translate(mat, transform_s.translate);
	//mat = glm::rotate(mat, angle_in_degrees, glm::vec3(x, y, z));
	mat = glm::scale(mat, transform_s.scale);

	return mat;
}
