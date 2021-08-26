#ifndef GLOABLE_CLASS_H
#define GLOABLE_CLASS_H

#include "VulkanCore/vulkan_core_headers.h"

#include <glm/glm.hpp>

namespace Gloable
{
	
	glm::vec3 ExtractEulerAngles(const glm::mat4& matrix);

	glm::vec3 ExtractTranslation(const glm::mat4& matrix);

	glm::mat4 ScaleTranslation(glm::mat4 matrix, float scale);
}
#endif // !GLOABLE_CLASS_H

