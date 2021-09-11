#include "Utils/GloableClass.h"
#include <iostream>
#include <fstream>

namespace Gloable
{
	bool FilpY = false;
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

	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	std::vector<float> preComputeLT;
	std::vector<float> preComputeL;

	void ReadpreComputeFile()
	{
		float data;
		std::ifstream ifs;
		ifs.open("../assets/precompute/light.txt");
		while (!ifs.eof()) {
			ifs >> data;
			preComputeL.emplace_back(data);
		}
		ifs.close();
		ifs.open("../assets/precompute/transport.txt");
		ifs >> data; // read head
		while (!ifs.eof()) {
			ifs >> data;
			preComputeLT.emplace_back(data);
		}
		ifs.close();
	}

	glm::vec3 GetPreComputeLT(size_t i, size_t j)
	{
		return glm::vec3(preComputeLT[i * 9 + j * 3], preComputeLT[i * 9 + j * 3 + 1], preComputeLT[i * 9 + j * 3 + 2]);
	}

	namespace SSAO
	{
		// SSAO
		std::default_random_engine rndEngine((unsigned)time(nullptr));
		std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);

		std::vector<glm::vec4> ssaoKernel(SSAO_KERNEL_SIZE);
		std::vector<glm::vec4> ssaoNoise(SSAO_NOISE_DIM* SSAO_NOISE_DIM);
		void InitSSAOKernel()
		{
			for (uint32_t i = 0; i < SSAO_KERNEL_SIZE; ++i)
			{
				glm::vec3 sample(rndDist(rndEngine) * 2.0 - 1.0, rndDist(rndEngine) * 2.0 - 1.0, rndDist(rndEngine));
				sample = glm::normalize(sample);
				sample *= rndDist(rndEngine);
				float scale = float(i) / float(SSAO_KERNEL_SIZE);
				scale = lerp(0.1f, 1.0f, scale * scale);
				ssaoKernel[i] = glm::vec4(sample * scale, 0.0f);
			}

			// Random noise

			for (uint32_t i = 0; i < static_cast<uint32_t>(ssaoNoise.size()); i++)
			{
				ssaoNoise[i] = glm::vec4(rndDist(rndEngine) * 2.0f - 1.0f, rndDist(rndEngine) * 2.0f - 1.0f, 0.0f, 0.0f);
			}
		}
	}
}


