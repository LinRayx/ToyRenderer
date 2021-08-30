#ifndef MATERIAL_BASE_H
#define MATERIAL_BASE_H

#include "VulkanCore/vulkan_core_headers.h"

#include <map>
#include "DynamicVariable/DynamicConstant.h"
#include <assimp/scene.h>

#include "Bindable/DepthStencilState.h";
#include "Bindable/VertexBuffer.h"
#include "Bindable/IndexBuffer.h"
#include "Drawable/MaterialBaseParent.h"

namespace Draw
{
	enum class MaterialType
	{
		Phone = 0,
		Outline,
		Skybox,
		PBR,
		ERROR,
	};
	class MaterialBase : public MaterialBaseParent
	{
	public:
		MaterialBase();


		void SetState(Bind::DepthStencilStateType type)
		{
			depthStencilType = type;
		}

		virtual MaterialType GetMaterailType()
		{
			return matType;
		}

		void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd);
		
		virtual void LoadModelTexture(const aiMaterial* material, string directory, string meshName) {}

		void BindMeshData(shared_ptr<Bind::VertexBuffer> vBuffer_ptr,
			shared_ptr<Bind::IndexBuffer> iBuffer_ptr);

		Bind::DepthStencilStateType depthStencilType = Bind::DepthStencilStateType::Default;
	protected:
	
		int loadTextures(const aiMaterial* mat, aiTextureType type, string directory, string meshName);

		string getTypeName(aiTextureType type);

		MaterialType matType = MaterialType::ERROR;

		shared_ptr<Bind::VertexBuffer> vBuffer_ptr;
		shared_ptr<Bind::IndexBuffer> iBuffer_ptr;
	};
}

#endif // !MATERIAL_BASE_H
