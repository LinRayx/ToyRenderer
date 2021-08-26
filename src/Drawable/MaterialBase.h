#ifndef MATERIAL_BASE_H
#define MATERIAL_BASE_H

#include "VulkanCore/vulkan_core_headers.h"

#include <map>
#include "DynamicVariable/DynamicConstant.h"
#include <assimp/scene.h>
#include "Drawable/Texture.h"

#include "Bindable/DepthStencilState.h";

namespace Draw
{
	enum class MaterialType
	{
		Phone = 0,
		Outline
	};
	class MaterialBase
	{
	public:
		MaterialBase();
		template<typename T>
		void SetValue(std::string key1, std::string key2, T value)
		{
			(*bufs[key1])[key2] = value;
		}
		void Update(int cur);

		void SetState(Bind::DepthStencilStateType type)
		{
			depthStencilType = type;
		}

		void Compile(shared_ptr<Graphics::DescriptorSetLayout> desc_layout_ptr);

		virtual void LoadModelTexture(const aiMaterial* material, string directory, string meshName) {}


		std::shared_ptr<Graphics::DescriptorSetCore> desc_ptr;

		Bind::DepthStencilStateType depthStencilType = Bind::DepthStencilStateType::Default;

		std::map<std::string, shared_ptr<Graphics::Buffer>> buffer_ptrs;
		std::map<std::string, shared_ptr<Dcb::Buffer>> bufs;
	protected:
		void addLayout(std::string key, Dcb::RawLayout&& layout,
			Graphics::LayoutType layoutType, Graphics::DescriptorType descType, Graphics::StageFlag stage);

		void addTexture(Graphics::LayoutType layout_type, Graphics::StageFlag stage, VkImageView textureImageView, VkSampler textureSampler);
		
		int loadTextures(const aiMaterial* mat, aiTextureType type, string directory, string meshName);

		string getTypeName(aiTextureType type);
	};
}

#endif // !MATERIAL_BASE_H
