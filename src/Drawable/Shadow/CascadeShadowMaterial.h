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
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd);
		void CollectVAndIBuffers(shared_ptr<Bind::VertexBuffer> v_ptr, shared_ptr<Bind::IndexBuffer> i_ptr);
	protected:
		virtual void initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo);
		virtual void UpdateSceneData();
		void SetDirectionLight(Control::DirectionLight& dl);
	private:
		void updateCascade();

		vector<shared_ptr<Bind::VertexBuffer> > vBuffer_ptrs;
		vector<shared_ptr<Bind::IndexBuffer> > iBuffer_ptrs;

		const int SHADOWMAP_DIM = 1024;

		float cascadeSplits[SHADOWMAP_COUNT];
		glm::mat4 cascadeViewProjMat[SHADOWMAP_COUNT];
		glm::mat4 inverseViewMat;
		uint32_t cascadeIndex;
		float cascadeSplitLambda = 0.95f;
		Control::DirectionLight* light_ptr;

		struct Cascade
		{
			float splitDepth;
			glm::mat4 viewProjMatrix;
		} cascades[SHADOWMAP_COUNT];

		VkViewport viewport = Graphics::initializers::viewportOffscreen((float)SHADOWMAP_DIM, (float)SHADOWMAP_DIM, 0.0f, 1.0f);
		VkRect2D scissor = Graphics::initializers::rect2D(SHADOWMAP_DIM, SHADOWMAP_DIM, 0, 0);
	};
}

#endif // !CASCADE_SHADOW_MATERIAL
