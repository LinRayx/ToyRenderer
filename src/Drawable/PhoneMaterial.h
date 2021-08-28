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
		virtual void Compile();
	private:
	};
}

#endif // !PHONE_MATERIAL_H
