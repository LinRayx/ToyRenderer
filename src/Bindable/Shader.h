#pragma once
#ifndef SHADER_H
#define SHADER_H
#include "VulkanCore/vulkan_core_headers.h"

#include <memory>
#include <string>

namespace Bind
{
	class Shader
	{
		friend class PixelShader;
		friend class VertexShader;
		//! Handles all information needed to compile a shader into a module
		typedef struct shader_request_s {
			//! A path to the file with the GLSL source code (relative to the CWD)
			const char* shader_file_path;
			//! The director(ies) which are searched for includes
			const char* include_path;
			//! The name of the function that serves as entry point
			const char* entry_point;
			//! A single bit from VkShaderStageFlagBits to indicate the targeted shader
			//! stage
			VkShaderStageFlags stage;
			//! Number of defines
			uint32_t define_count;
			//! A list of strings providing the defines, either as "IDENTIFIER" or
			//! "IDENTIFIER=VALUE". Do not use white space, these strings go into the
			//! command line unmodified.
			char** defines;
		} shader_request_t;


		//! Bundles a Vulkan shader module with its SPIRV code
		typedef struct shader_s {
			//! The Vulkan shader module
			VkShaderModule module;
			//! The size of the compiled SPIRV code in bytes
			size_t spirv_size;
			//! The compiled SPIRV code
			uint32_t* spirv_code;
		} shader_t;
	public:
		Shader()  {}
		void CompileShader(std::string shader_file_path, std::string include_path, std::string entry_point, VkShaderStageFlagBits stage, 
			shader_t* shader);
	private:
		int compile_glsl_shader(shader_t* shader, const shader_request_t* request);
		const char* get_shader_stage_name(VkShaderStageFlags stage);
		int compile_glsl_shader_with_second_chance(shader_t* shader, const shader_request_t* request);
		void destroy_shader(shader_t* shader);

	private:

		
	};
}

#endif // !SHADER_H