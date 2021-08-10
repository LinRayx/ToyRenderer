#pragma once
#ifndef PIXEL_SHADER_H
#define PIXEL_SHADER_H

#include "Bindable.h"
#include "Shader.h"
#include "Vulkan.h"
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
		PixelShader(std::shared_ptr<Graphics::Vulkan> vulkan_ptr, std::string shader_file_path, std::string include_path, std::string entry_point)
		{
			shaderBuild = std::make_shared<Shader>(vulkan_ptr);
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
