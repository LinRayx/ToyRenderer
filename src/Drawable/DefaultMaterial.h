#ifndef DEFAULT_MATERIAL_H
#define DEFAULT_MATERIAL_H

#include "Drawable/MaterialBase.h"
namespace Draw
{
	class DefaultMaterial : public MaterialBase
	{
	public:
		DefaultMaterial(glm::vec4 color = glm::vec4(1, 1, 1, 1));

		~DefaultMaterial();
		virtual void Compile();
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd);
	private:
		Dcb::Buffer* pushBlock;
		glm::vec4 color = glm::vec4(1, 1, 1, 1);
	};
}

#endif // !DEFAULT_MATERIAL_H
