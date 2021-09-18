#include "Drawable/PostProcess/CompositionMaterial.h"

namespace Draw
{
	CompositionMaterial::CompositionMaterial() : MaterialBase()
	{
		using namespace Graphics;
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, GetImageView("diffuseMap"), GetSampler("diffuseMap"));
		addTexture(LayoutType::SCENE, StageFlag::FRAGMENT, GetImageView("specularMap"), GetSampler("specularMap"));
	}
	void CompositionMaterial::BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd)
	{
		MaterialBaseParent::BuildCommandBuffer(cmd);
	}
	void CompositionMaterial::initPipelineCreateInfo(VkGraphicsPipelineCreateInfo& pinfo)
	{
		cout << "CompositionMaterial::initPipelineCreateInfo" << endl;
		pinfo.renderPass = Graphics::nameToRenderPass[Graphics::RenderPassType::Default]->renderPass;
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::FULLSCREEN_VERT, VK_SHADER_STAGE_VERTEX_BIT, std::move(vert_defs)));
		shaderStages.emplace_back(Bind::CreateShaderStage(Bind::ShaderType::COMPOSITION, VK_SHADER_STAGE_FRAGMENT_BIT, std::move(frag_defs)));
		pinfo.pVertexInputState = &emptyVertexInputState;
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		depthStencilState.depthTestEnable = VK_FALSE;
	}
}