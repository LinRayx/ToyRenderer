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
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd);
		virtual glm::vec3 GetPosition();
		virtual bool SetUI();
		void SetColor(glm::vec4 color);
		glm::vec4 GetColor();
	protected:
		virtual void initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo);
	private:
		Dcb::Buffer* pushBlock;
		glm::vec4 color = glm::vec4(1, 1, 1, 1);
	};
}

#endif // !DEFAULT_MATERIAL_H
