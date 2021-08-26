#pragma once
#ifndef PIXEL_SHADER_H
#define PIXEL_SHADER_H

#include "Bindable.h"
#include "Shader.h"
#include "VulkanCore/vulkan_core_headers.h"

#include <string>
#include <memory>

namespace Draw
{
	class Drawable;
}

namespace Bind
{
	class PixelShader : public Bindable
	{
		friend class Draw::Drawable;
	public:
		PixelShader(std::string shader_file_path, std::string include_path, std::string entry_point)
		{
			shaderBuild = std::make_shared<Shader>();
			shaderBuild->CompileShader(shader_file_path, include_path, entry_point, VK_SHADER_STAGE_FRAGMENT_BIT, &shader);
		}
		~PixelShader()
		{
			shaderBuild->destroy_shader(&shader);
		}

	private:
		std::shared_ptr<Shader> shaderBuild;
	public:
		Shader::shader_t shader;
	};
}

#endif // !PIXEL_SHADER_H
