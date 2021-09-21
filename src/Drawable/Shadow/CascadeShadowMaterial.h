#ifndef CASCADE_SHADOW_MATERIAL
#define CASCADE_SHADOW_MATERIAL

#include "Drawable/MaterialBase.h"
#include "Scene/DirectionLight.h"

namespace Draw
{

	class CascadeShadowMaterial : public MaterialBase
	{
	public:
		CascadeShadowMaterial();

		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd, int index, int face);
		void SetDirectionLight(Control::DirectionLight& dl);
		virtual void UpdateSceneData();

	protected:
		virtual void initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo);
	private:
		const int SHADOWMAP_DIM = Gloable::CSM_MAP_DIM;
		Control::DirectionLight* light_ptr;

		VkViewport viewport = Graphics::initializers::viewportOffscreen((float)SHADOWMAP_DIM, (float)SHADOWMAP_DIM, 0.0f, 1.0f);
		VkRect2D scissor = Graphics::initializers::rect2D(SHADOWMAP_DIM, SHADOWMAP_DIM, 0, 0);
	};

	namespace Cascades
	{
		void updateCascade();
		glm::vec4 GetCascadeSplits();
		glm::mat4* GetCascadeViewProjMat();
		glm::mat4 GetInverseViewMat();
		float& GetCascadeSplitLambda();
	}
}

#endif // !CASCADE_SHADOW_MATERIAL
