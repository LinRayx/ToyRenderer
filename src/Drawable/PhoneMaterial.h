#ifndef PHONE_MATERIAL_H
#define PHONE_MATERIAL_H

#include "Drawable/MaterialBase.h"


namespace Draw
{
	class PhoneMaterial : public MaterialBase
	{
	public:
		PhoneMaterial();
		virtual void LoadModelTexture(const aiMaterial* material, string directory, string meshName) override;
	private:
	};
}

#endif // !PHONE_MATERIAL_H
