#ifndef MATERIAL_BASE_H
#define MATERIAL_BASE_H

#include "VulkanCore/vulkan_core_headers.h"

#include "DynamicVariable/DynamicConstant.h"
#include <assimp/scene.h>
#include <assimp/pbrmaterial.h>

#include "Drawable/MaterialBaseParent.h"
#include "Bindable/DepthStencilState.h"

namespace Draw
{
	enum class MaterialType
	{
		Phone = 0,
		Outline,
		Skybox,
		PBR,
		GBuffer,
		FS_SSAO,
		FS_BLUR,
		PBR_Deferred,
		DEFAULT,
		POINTLIGHT,
		OMNISHADOW,
		ERROR,
		
	};
	class MaterialBase : public MaterialBaseParent
	{
	public:
		MaterialBase();

		MaterialBase(bool flag);

		virtual MaterialType GetMaterailType()
		{
			return matType;
		}

		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd);
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd, int index, int face) {}
		virtual void LoadModelTexture(const aiMaterial* material, string directory, string meshName) {}

		void SetModelName(string name);
		virtual void UpdateSceneData();
		virtual void SetTransform(glm::mat4 translate, glm::mat4 rotate);
	protected:
		int loadTextures(const aiMaterial* mat, aiTextureType type, string directory, string meshName);
		string getTypeName(aiTextureType type);
		MaterialType matType = MaterialType::ERROR;
		string modelName;

	};
}

#endif // !MATERIAL_BASE_H
