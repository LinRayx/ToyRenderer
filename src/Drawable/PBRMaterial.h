#pragma once
#ifndef PBR_MATERIAL_H
#define PBR_MATERIAL_H

#include "Drawable/MaterialBase.h"

namespace Draw
{
	class PBRMaterial : public MaterialBase
	{
	public:
		PBRMaterial();
		virtual void LoadModelTexture(const aiMaterial* material, string directory, string meshName);
		virtual void Compile();
	private:
		void AddCubeTexture(string cube_texture_name);
	};
}

#endif // !PBR_MATERIAL_H
