#pragma once
#ifndef SHADER_H
#define SHADER_H
#include "VulkanCore/vulkan_core_headers.h"

#include <memory>
#include <string>
#include <map>

namespace Bind
{

	enum class ShaderType
	{
		Phone,
		Outline,
		Skybox,
		PBR,
		BRDFLUT,
		IRRADIANCE,
		PREFILTER,
		UI,
		GBUFFER,
		FULLSCREEN_VERT,
		SSAO,
		EMPTY,
	};

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

	public:
		typedef struct shader_s {
			//! The Vulkan shader module
			VkShaderModule module;
			//! The size of the compiled SPIRV code in bytes
			size_t spirv_size;
			//! The compiled SPIRV code
			uint32_t* spirv_code;
		} shader_t;
		Shader()  {}
		Shader(std::string shader_file_path, std::string include_path, std::string entry_point, VkShaderStageFlagBits stage);
		~Shader();
		void CompileShader(std::string shader_file_path, std::string include_path, std::string entry_point, VkShaderStageFlagBits stage, 
			shader_t* shader);

		VkShaderModule GetShaderModule();

	private:
		int compile_glsl_shader(shader_t* shader, const shader_request_t* request);
		const char* get_shader_stage_name(VkShaderStageFlags stage);
		int compile_glsl_shader_with_second_chance(shader_t* shader, const shader_request_t* request);
		void destroy_shader(shader_t* shader);

	private:
		shader_t shader;
		
	};

	struct ShaderData
	{
		ShaderData()  {}
		ShaderData(Shader* vert_shader, string frag_name)
		{
			this->vert_shader = vert_shader;
			frag_shader = new Shader("../src/shaders/" + frag_name + ".frag.glsl", "../src/shaders", "main", VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		void operator=(const ShaderData& sd)
		{
			vert_shader = sd.vert_shader;
			frag_shader = sd.frag_shader;
		}

		ShaderData(string name, bool only_vert = false)
		{
			vert_shader = new Shader("../src/shaders/" + name + ".vert.glsl", "../src/shaders", "main", VK_SHADER_STAGE_VERTEX_BIT);
			if(!only_vert) frag_shader = new Shader("../src/shaders/" + name + ".frag.glsl", "../src/shaders", "main", VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		~ShaderData()
		{
			if (vert_shader != nullptr)
				delete vert_shader;
			if (frag_shader != nullptr)
				delete frag_shader;
		}
		Shader* vert_shader = nullptr;
		Shader* frag_shader = nullptr;
	};

	extern std::map<ShaderType, unique_ptr<ShaderData>> shaderFactory;
	void LoadShaders();
}

#endif // !SHADER_H