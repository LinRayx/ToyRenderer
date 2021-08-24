#include "Drawable/MaterialBase.h"

namespace Draw
{
	MaterialBase::MaterialBase(std::shared_ptr<Graphics::Vulkan> vulkan_ptr, std::shared_ptr<Graphics::DescriptorPool> desc_pool_ptr)
		: vulkan_ptr(vulkan_ptr)
	{
		desc_ptr = std::make_shared<Graphics::DescriptorSetCore>(vulkan_ptr, desc_pool_ptr);
		Dcb::RawLayout layout;
		layout.Add<Dcb::Matrix>("viewMat");
		layout.Add<Dcb::Matrix>("projMat");

		Dcb::RawLayout layout2;
		layout2.Add<Dcb::Float3>("viewPos");
		layout2.Add<Dcb::Float3>("direLightDir");
		layout2.Add<Dcb::Float3>("direLightColor");

		addLayout("ViewAndProj", std::move(layout), Graphics::LayoutType::SCENE, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::VERTEX);
		addLayout("Light", std::move(layout2), Graphics::LayoutType::SCENE, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::FRAGMENT);

		Dcb::RawLayout transBuf;
		transBuf.Add<Dcb::Matrix>("modelTrans");
		addLayout("Model", std::move(transBuf), Graphics::LayoutType::MODEL, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::VERTEX);
	}

	void MaterialBase::Update(int cur)
	{
		for (auto it : buffer_ptrs)
		{
			it.second->UpdateData(cur, bufs[it.first]->GetSizeInBytes(), bufs[it.first]->GetData());
		}
	}
	void MaterialBase::Compile(shared_ptr<Graphics::DescriptorSetLayout> desc_layout_ptr)
	{
		desc_ptr->Compile(desc_layout_ptr);
	}
	void MaterialBase::addLayout(std::string key, Dcb::RawLayout&& layout, Graphics::LayoutType layoutType, Graphics::DescriptorType descType, Graphics::StageFlag stage)
	{
		bufs[key] = make_shared<Dcb::Buffer>(std::move(layout));
		buffer_ptrs[key] = make_shared<Graphics::Buffer>(vulkan_ptr, Graphics::BufferUsage::UNIFORM, bufs[key]->GetSizeInBytes());
		desc_ptr->Add(layoutType, descType, stage, buffer_ptrs[key]);
	}
	void MaterialBase::addTexture(Graphics::LayoutType layout_type, Graphics::StageFlag stage, VkImageView textureImageView, VkSampler textureSampler)
	{
		desc_ptr->Add(layout_type, Graphics::DescriptorType::TEXTURE2D, stage, textureImageView, textureSampler);
	}
	int MaterialBase::loadTextures(const aiMaterial* mat, aiTextureType type, string directory, string meshName)
	{
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			Draw::textureManager->CreateTexture(directory + str.C_Str(), meshName + "_" + getTypeName(type));
			addTexture(Graphics::LayoutType::MODEL, Graphics::StageFlag::FRAGMENT, Draw::textureManager->nameToTex[meshName].textureImageView, Draw::textureManager->nameToTex[meshName].textureSampler);
		}
		SetValue("TextureFlags", "Has" + getTypeName(type) + "Tex", mat->GetTextureCount(type) > 0);
		return mat->GetTextureCount(type);
	}
	string MaterialBase::getTypeName(aiTextureType type)
	{
		switch (type)
		{
		case aiTextureType_NONE:
			return "NONE";
		case aiTextureType_DIFFUSE:
			return "Diffuse";
		case aiTextureType_SPECULAR:
			return "Specular";
		case aiTextureType_AMBIENT:
			return "ambient";
		case aiTextureType_EMISSIVE:
			return "emissive";
		case aiTextureType_HEIGHT:
			return "height";
		case aiTextureType_NORMALS:
			return "normal";
		case aiTextureType_SHININESS:
			return "shininess";
		case aiTextureType_OPACITY:
			return "opacity";
		case aiTextureType_DISPLACEMENT:
			return "displacement";
		case aiTextureType_LIGHTMAP:
			return "lightmap";
		case aiTextureType_REFLECTION:
			return "reflection";
		case aiTextureType_BASE_COLOR:
			return "basecolor";
		case aiTextureType_NORMAL_CAMERA:
			return "normalcamera";
		case aiTextureType_EMISSION_COLOR:
			return "emissioncolor";
		case aiTextureType_METALNESS:
			return "metealness";
		case aiTextureType_DIFFUSE_ROUGHNESS:
			return "roughness";
		case aiTextureType_AMBIENT_OCCLUSION:
			return "ao";
		case aiTextureType_UNKNOWN:
			break;
		case _aiTextureType_Force32Bit:
			break;
		default:
			break;
		}
	}
}

