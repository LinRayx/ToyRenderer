#ifndef PHONE_MATERIAL_H
#define PHONE_MATERIAL_H

#include "Drawable/MaterialBase.h"


namespace Draw
{
	class PhoneMaterial : public MaterialBase
	{
	public:
		PhoneMaterial(std::shared_ptr<Graphics::Vulkan> vulkan_ptr, std::shared_ptr<Graphics::DescriptorPool> desc_pool_ptr);
		virtual void LoadModelTexture(const aiMaterial* material, string directory, string meshName) override;
	private:
	};
}

#endif // !PHONE_MATERIAL_H
