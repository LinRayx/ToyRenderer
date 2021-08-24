#include "Drawable/PhoneMaterial.h"

namespace Draw
{
	PhoneMaterial::PhoneMaterial(std::shared_ptr<Graphics::Vulkan> vulkan_ptr, std::shared_ptr<Graphics::DescriptorPool> desc_pool_ptr)
		: MaterialBase(vulkan_ptr, desc_pool_ptr)
	{
		Dcb::RawLayout textureFlags;
		textureFlags.Add<Dcb::Bool>("HasDiffuseTex");
		textureFlags.Add<Dcb::Bool>("HasSpecularTex");
		addLayout("TextureFlags", std::move(textureFlags), Graphics::LayoutType::MODEL, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::FRAGMENT);
	}
	void PhoneMaterial::LoadModelTexture(const aiMaterial* material, string directory, string meshName)
	{
		loadTextures(material, aiTextureType_DIFFUSE, directory, meshName);
		loadTextures(material, aiTextureType_SPECULAR, directory, meshName);
	}
}