#ifndef SKYBOX_MATERIAL_H
#define SKYBOX_MATERIAL_H

#include "VulkanCore/vulkan_core_headers.h"
#include "Drawable/MaterialBase.h"

namespace Draw
{
	class SkyboxMaterial : public MaterialBase
	{
	public:
		SkyboxMaterial();
		void AddCubeTexture(string skybox_texture_name);

		virtual void Compile();
		virtual void UpdateSceneData();

	private:	
	};
}

#endif // !SKYBOX_MATERIAL_H
