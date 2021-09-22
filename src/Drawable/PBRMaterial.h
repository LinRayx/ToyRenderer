#pragma once
#ifndef PBR_MATERIAL_H
#define PBR_MATERIAL_H

#include "Drawable/MaterialBase.h"

namespace Draw
{
	enum class PBRTEXTURE_TYPE
	{
		ALBEDO,
		NORMAL,
		METALLIC,
		ROUGHNESS,
	};
	class PBRMaterial : public MaterialBase
	{
	public:
		PBRMaterial(bool flag);
		PBRMaterial();
		virtual void LoadModelTexture(const aiMaterial* material, string directory, string meshName);
		virtual void UpdateSceneData();
		virtual bool SetUI();
		float& GetMetallic();
		glm::vec3& GetAlbedo();
		float& GetRoughness();
		void AddTexture(PBRTEXTURE_TYPE type, string name);
	protected:
		virtual void initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo);
		void InitPBRData();
		void addPBRTexture(string name);
		float metallic = 0.1f;
		glm::vec3 albedo = glm::vec3(1, 0, 0);
		uint32_t pbrbinding = 1;
		float roughness = 1.0f;
	};
}

#endif // !PBR_MATERIAL_H
