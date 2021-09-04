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
		PBR_Deferred,
		DEFAULT,
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
		virtual void LoadModelTexture(const aiMaterial* material, string directory, string meshName) {}
		void BindMeshData(shared_ptr<Bind::VertexBuffer> vBuffer_ptr,
			shared_ptr<Bind::IndexBuffer> iBuffer_ptr);
		void SetModelName(string name);
		virtual void UpdateSceneData();
	protected:
		int loadTextures(const aiMaterial* mat, aiTextureType type, string directory, string meshName);
		string getTypeName(aiTextureType type);
		MaterialType matType = MaterialType::ERROR;
		string modelName;

	};
}

#endif // !MATERIAL_BASE_H
