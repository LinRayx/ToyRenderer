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

	void createFullScreenRenderPass(string name, Graphics::RenderPassType rpType)
	{
		Graphics::RenderPass* rp6 = new Graphics::RenderPass();
		rp6->CreateFullScreenRenderPass(
			textureManager->nameToTex[name].format,
			textureManager->nameToTex[name].textureImageView,
			textureManager->nameToTex[name].texWidth,
			textureManager->nameToTex[name].texHeight
		);
		Graphics::nameToRenderPass[rpType] = rp6;
	}

	void CreateRenderPass()
	{
		using namespace Graphics;
		RenderPass* rp = new RenderPass();
		rp->CreateRenderPass(textureManager->nameToTex["GBuffer_depth"].textureImageView);
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

		createFullScreenRenderPass("ssaoMap", RenderPassType::FULLSCREEN_SSAO);
		createFullScreenRenderPass("ssaoBlurMap", RenderPassType::FULLSCREEN_BLUR);

		RenderPass* rp6 = new RenderPass();
		rp6->CreateShadowMappingRenderPass(
			textureManager->nameToTex["omni_color_attachment"].format,
			textureManager->nameToTex["omni_color_attachment"].textureImageView,
			textureManager->nameToTex["omni_color_attachment"].texWidth,
			textureManager->nameToTex["omni_color_attachment"].texHeight,
			textureManager->nameToTex["omni_depth_attachment"].textureImageView
		);
		nameToRenderPass[RenderPassType::ONMISHADOW] = rp6;

		RenderPass* rp7 = new RenderPass();
		rpdatas.clear();
		rpdatas.emplace_back(RenderPass::RpData{ textureManager->nameToTex["diffuseMap"].format, textureManager->nameToTex["diffuseMap"].textureImageView });
		rpdatas.emplace_back(RenderPass::RpData{ textureManager->nameToTex["specularMap"].format, textureManager->nameToTex["specularMap"].textureImageView });
		rp7->CreateLightPass(rpdatas, Vulkan::getInstance()->GetWidth(), Vulkan::getInstance()->GetHeight());
		nameToRenderPass[RenderPassType::LIGHT] = rp7;
	}


}