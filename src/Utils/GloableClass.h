#ifndef GLOABLE_CLASS_H
#define GLOABLE_CLASS_H

#include <glm/glm.hpp>
#include <time.h>
#include <random>
#include <array>

namespace Gloable
{
	
	glm::vec3 ExtractEulerAngles(const glm::mat4& matrix);

	glm::vec3 ExtractTranslation(const glm::mat4& matrix);

	glm::mat4 ScaleTranslation(glm::mat4 matrix, float scale);

	extern bool FilpY;

	namespace SSAO
	{
		const size_t SSAO_NOISE_DIM = 4;
		const size_t SSAO_KERNEL_SIZE = 32;
		const float SSAO_RADIUS = 0.3f;

		// Sample kernel
		extern std::vector<glm::vec4> ssaoKernel;
		extern std::vector<glm::vec4> ssaoNoise;

		void InitSSAOKernel();

	}

	void ReadpreComputeFile();
	glm::vec3 GetPreComputeLT(size_t i, size_t j);
}
#endif // !GLOABLE_CLASS_H

