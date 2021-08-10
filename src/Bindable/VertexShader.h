#pragma once
#ifndef VERTEX_SHADER_H
#define VERTEX_SHADER_H

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
	class VertexShader : public Bindable
	{
		friend class Draw::Drawable;
	public:
		VertexShader(std::shared_ptr<Graphics::Vulkan> vulkan_ptr, std::string shader_file_path, std::string include_path, std::string entry_point)
		{
			shaderBuild = std::make_shared<Shader>(vulkan_ptr);
			shaderBuild->CompileShader(shader_file_path, include_path, entry_point, VK_SHADER_STAGE_VERTEX_BIT, &shader);
		}
		~VertexShader()
		{
			shaderBuild->destroy_shader(&shader);
		}

		std::shared_ptr<Shader> shaderBuild;
	public:
		Shader::shader_t shader;
	};
}

#endif // !VERTEX_SHADER_H
