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
		DEFAULT,
		PBR_Deferred,
		BLUR,
		OMNISHADOW,
		COMPOSITION,
		CASCADE_SHADOW,
		EMPTY,
	};

	class Shader
	{
		friend class PixelShader;
		friend class VertexShader;
		//! Handles all information needed to compile a shader into a module
		typedef struct shader_request_s {
			//! A path to the file with the GLSL source code (relative to the CWD)
			string shader_file_path;
			//! The director(ies) which are searched for includes
			const char* include_path;
			//! The name of the function that serves as entry point
			const char* entry_point;
			//! A single bit from VkShaderStageFlagBits to indicate the targeted shader
			//! stage
			VkShaderStageFlags stage;
			//! Number of defines
			vector<string> defines;
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
		Shader(std::string shader_file_path, std::string include_path, std::string entry_point, VkShaderStageFlagBits stage, vector<string>&& defines);

		Shader(std::string shader_file_path, std::string include_path, std::string entry_point, VkShaderStageFlagBits stage);
		~Shader();

		VkShaderModule GetShaderModule();

	private:
		int compile_glsl_shader(const shader_request_t* request);
		const char* get_shader_stage_name(VkShaderStageFlags stage);
		int compile_glsl_shader_with_second_chance(const shader_request_t* request);
		void destroy_shader();

	private:
		unique_ptr<shader_t> shader;
		
	};

	struct ShaderData
	{
		ShaderData()  {}
		ShaderData(shared_ptr<Shader> vert_shader, string frag_name)
		{
			this->vert_shader = vert_shader;
			frag_shader = make_shared<Shader>("../src/shaders/" + frag_name + ".frag.glsl", "../src/shaders", "main", VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		void operator=(const ShaderData& sd)
		{
			vert_shader = sd.vert_shader;
			frag_shader = sd.frag_shader;
		}

		ShaderData(string name, bool has_frag = true, bool has_gemo = false)
		{
			vert_shader = make_shared<Shader>("../src/shaders/" + name + ".vert.glsl", "../src/shaders", "main", VK_SHADER_STAGE_VERTEX_BIT);
			if (has_frag) {
				frag_shader = make_shared<Shader>("../src/shaders/" + name + ".frag.glsl", "../src/shaders", "main", VK_SHADER_STAGE_FRAGMENT_BIT);
			}
			if (has_gemo)
				gemo_shader = make_shared<Shader>("../src/shaders/" + name + ".geom.glsl", "../src/shaders", "main", VK_SHADER_STAGE_GEOMETRY_BIT);
		}
		~ShaderData()
		{
		}

		shared_ptr<Shader> vert_shader = nullptr;
		shared_ptr<Shader>  frag_shader = nullptr;
		shared_ptr<Shader>  gemo_shader = nullptr;
	};

	extern std::map<ShaderType, unique_ptr<ShaderData>> shaderFactory;
	void LoadShaders();
	void LoadShaderPaths();
	VkPipelineShaderStageCreateInfo CreateShaderStage(ShaderType type, VkShaderStageFlagBits stage, vector<string>&& defs);
}

#endif // !SHADER_H