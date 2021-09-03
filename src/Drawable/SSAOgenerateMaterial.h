#ifndef SSAO_GENERATE_MATERIAL
#define SSAO_GENERATE_MATERIAL

#include "Drawable/MaterialBaseParent.h"
#include "Utils/GloableClass.h"

namespace Draw
{
	class SSAOgenerateMaterial : public MaterialBaseParent
	{
	public:
		SSAOgenerateMaterial();
		virtual void Compile();
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd);
		virtual void UpdateSceneData();
	};
}

#endif