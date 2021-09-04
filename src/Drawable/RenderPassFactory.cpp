#include "Drawable/RenderPassFactory.h"

namespace Draw
{
	void createOffScreenRenderPass(string name, Graphics::RenderPassType type, VkImageLayout finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		Graphics::RenderPass* rp4 = new Graphics::RenderPass();
		rp4->CreateOffScreenRenderPass(
			textureManager->nameToTex[name].format,
			textureManager->nameToTex[name].textureImageView,
			textureManager->nameToTex[name].texWidth,
			textureManager->nameToTex[name].texHeight,
			finalLayout);
		Graphics::nameToRenderPass[type] = rp4;
	}

	void createFullScreenRenderPass(string name)
	{
		Graphics::RenderPass* rp6 = new Graphics::RenderPass();
		rp6->CreateFullScreenRenderPass(
			textureManager->nameToTex[name].format,
			textureManager->nameToTex[name].textureImageView,
			textureManager->nameToTex[name].texWidth,
			textureManager->nameToTex[name].texHeight
		);
		Graphics::nameToRenderPass[Graphics::RenderPassType::FULLSCREEN_SSAO] = rp6;
	}

	void CreateRenderPass()
	{
		using namespace Graphics;
		RenderPass* rp = new RenderPass();
		rp->CreateRenderPass(textureManager->nameToTex["depth"].textureImageView);
		nameToRenderPass[RenderPassType::Default] = rp;

		createOffScreenRenderPass("brdf_lut", RenderPassType::BRDFLUT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		createOffScreenRenderPass("irradiance_attachment", RenderPassType::IRRADIANCE);
		createOffScreenRenderPass("prefilter_attachment", RenderPassType::PREFILTER);

		RenderPass* rp5 = new RenderPass();
		vector<RenderPass::RpData> rpdatas;
		rpdatas.emplace_back(RenderPass::RpData{ textureManager->nameToTex["GBuffer_position"].format, textureManager->nameToTex["GBuffer_position"].textureImageView });
		rpdatas.emplace_back(RenderPass::RpData{ textureManager->nameToTex["GBuffer_normals"].format, textureManager->nameToTex["GBuffer_normals"].textureImageView });
		rpdatas.emplace_back(RenderPass::RpData{ textureManager->nameToTex["GBuffer_albedo"].format, textureManager->nameToTex["GBuffer_albedo"].textureImageView });
		rpdatas.emplace_back(RenderPass::RpData{ textureManager->nameToTex["GBuffer_metallic_roughness"].format, textureManager->nameToTex["GBuffer_metallic_roughness"].textureImageView });
		rpdatas.emplace_back(RenderPass::RpData{ textureManager->nameToTex["GBuffer_depth"].format, textureManager->nameToTex["GBuffer_depth"].textureImageView });

		rp5->CreateDeferredRenderPass(rpdatas);
		nameToRenderPass[RenderPassType::DEFERRED] = rp5;

		createFullScreenRenderPass("ssaoMap");
	}


}