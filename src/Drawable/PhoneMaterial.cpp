#include "Drawable/PhoneMaterial.h"

namespace Draw
{
	PhoneMaterial::PhoneMaterial() : MaterialBase()
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