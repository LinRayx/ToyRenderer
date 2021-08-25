#include "Utils/GloableClass.h"

namespace Gloable
{
	glm::vec3 ExtractEulerAngles(const glm::mat4& mat)
	{
		glm::vec3 euler;
		
		euler.x = asinf(-mat[2][1]);                  // Pitch
		if (cosf(euler.x) > 0.0001)                // Not at poles
		{
			euler.y = atan2f(mat[2][0], mat[2][2]);      // Yaw
			euler.z = atan2f(mat[0][1], mat[1][1]);      // Roll
		}
		else
		{
			euler.y = 0.0f;                           // Yaw
			euler.z = atan2f(-mat[1][0], mat[0][0]);     // Roll
		}

		return euler;
	}
	glm::vec3 ExtractTranslation(const glm::mat4& matrix)
	{
		return glm::vec3(matrix[3][0], matrix[3][1], matrix[3][2]);
	}
	glm::mat4 ScaleTranslation(glm::mat4 matrix, float scale)
	{
		matrix[3][0] *= scale;
		matrix[3][1] *= scale;
		matrix[3][2] *= scale;
		return matrix;
	}
}