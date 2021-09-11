#ifndef ONMI_SHADOW_GENERATER_MATERIAL_H
#define ONMI_SHADOW_GENERATER_MAERTIAL_H

#include "Drawable/MaterialBase.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Draw
{
	class OnmiShadowGenMaterial : public MaterialBase
	{
	public:
		OnmiShadowGenMaterial();
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd, int index, int face);
		virtual void UpdateSceneData();
		virtual void SetTransform(glm::mat4 translate, glm::mat4 rotate);
	protected:
		virtual void initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo) override;
	private:
		int dim = 1024;
		std::vector<glm::mat4> shadowTransforms;
		Control::PointLight* pl_ptr = &Control::Scene::getInstance()->pointLights[0];
		float farPlane = Control::Scene::getInstance()->camera_ptr->GetFarPlane();
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.f, 0.1f, farPlane);
		Dcb::Buffer* pushBlock;
		string attachment = "omni_color_attachment";
		string resource = "omni_depth_map";

		VkViewport viewport = Graphics::initializers::viewportOffscreen((float)dim, (float)dim, 0.0f, 1.0f);
		VkRect2D scissor = Graphics::initializers::rect2D(dim, dim, 0, 0);
	};
}

#endif // !ONMI_SHADOW_GENERATER_H
