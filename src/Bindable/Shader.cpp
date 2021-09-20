#pragma once
#include "Shader.h"
#include "string_utilities.h"
#include <stdarg.h>

namespace Bind
{
	Shader::Shader(std::string shader_file_path, std::string include_path, std::string entry_point, VkShaderStageFlagBits stage, vector<string>&& defines)
	{
		shader = make_unique<shader_t>();
		shader_request_t shader_request = {};
		shader_request.shader_file_path = (shader_file_path + "." + get_shader_stage_name(stage) + ".glsl");
		shader_request.include_path = include_path.c_str();
		shader_request.entry_point = entry_point.c_str();
		shader_request.stage = stage;
		shader_request.defines = defines;

		if (compile_glsl_shader_with_second_chance(&shader_request)) {
			exit(1);
		}
	}
	Shader::Shader(std::string shader_file_path, std::string include_path, std::string entry_point, VkShaderStageFlagBits stage)
	{
		shader = make_unique<shader_t>();
		shader_request_t shader_request = {};
		shader_request.shader_file_path = shader_file_path.c_str();
		shader_request.include_path = include_path.c_str();
		shader_request.entry_point = entry_point.c_str();
		shader_request.stage = stage;

		if (compile_glsl_shader_with_second_chance(&shader_request)) {
			exit(1);
		}
	}
	Shader::~Shader()
	{
		destroy_shader();
	}

	VkShaderModule Shader::GetShaderModule()
	{
		return shader->module;
	}
	int Shader::compile_glsl_shader(const shader_request_t* request) {
		auto device = Graphics::Vulkan::getInstance()->GetDevice();
		if (!get_shader_stage_name(request->stage)) {
			printf("Invalid stage specification %u passed for shader %s.", request->stage, request->shader_file_path.c_str());
			return 1;
		}
		//char buf[80];
		//getcwd(buf, sizeof(buf));
		//printf("current working directory: %s\n", buf);

		// Verify that the shader file exists by opening and closing it
#ifndef NDEBUG
		FILE* shader_file = fopen(request->shader_file_path.c_str(), "r");
		if (!shader_file) {
			printf("The shader file at path %s does not exist or cannot be opened.\n", request->shader_file_path.c_str());
			return 1;
		}
		fclose(shader_file);
#endif
		// Delete the prospective output file such that we can verify its existence
		// to see if the compiler did anything
		const char* spirv_path_pieces[] = { request->shader_file_path.c_str(), ".spv" };
		char* spirv_path = concatenate_strings(COUNT_OF(spirv_path_pieces), spirv_path_pieces);
		remove(spirv_path);
		// Build the part of the command line for defines

		string concatenated_defines = "";
		for (auto& str : request->defines) {
			concatenated_defines += "--D " + str + " ";
		}
		// Construct the command line
		const char* command_line_pieces[] = {
			"glslangValidator -V100 --target-env spirv1.5 ",
			"-S ", get_shader_stage_name(request->stage),
	#ifndef NDEBUG
			" -g -Od ",
	#endif
			concatenated_defines.c_str(),
			" -I\"", request->include_path, "\" ",
			"--entry-point ", request->entry_point,
			" -o \"", spirv_path,
			"\" \"", request->shader_file_path.c_str(), "\""
		};
		char* command_line = concatenate_strings(COUNT_OF(command_line_pieces), command_line_pieces);
		// Check whether command processing is available at all
#ifndef NDEBUG
		if (!system(NULL)) {
			printf("No command processor is available. Cannot invoke the compiler to compile the shader at path %s.\n", request->shader_file_path.c_str());
			free(command_line);
			free(spirv_path);
			return 1;
		}
#endif
		// Invoke the command line and see whether it produced an output file
		system(command_line);
		FILE* file = fopen(spirv_path, "rb");
		if (!file) {
			printf("glslangValidator failed to compile the shader at path %s. The full command line is:\n%s\n", request->shader_file_path.c_str(), command_line);
			free(command_line);
			free(spirv_path);
			return 1;
		}
		free(command_line);
		free(spirv_path);
		// Read the SPIR-V code from the file
		if (fseek(file, 0, SEEK_END) || (shader->spirv_size = ftell(file)) < 0) {
			printf("Failed to determine the file size for the compiled shader %s.", spirv_path);
			fclose(file);
			return 1;
		}
		shader->spirv_code = (uint32_t*)malloc(shader->spirv_size);
		fseek(file, 0, SEEK_SET);
		shader->spirv_size = fread(shader->spirv_code, sizeof(char), shader->spirv_size, file);
		fclose(file);
		// Create the Vulkan shader module
		VkShaderModuleCreateInfo module_info = {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = shader->spirv_size,
			.pCode = shader->spirv_code
		};
		if (vkCreateShaderModule(device.device, &module_info, device.allocator, &shader->module)) {
			printf("Failed to create a shader module from %s.\n", request->shader_file_path.c_str());
			destroy_shader();
		}
		return 0;
	}


	int Shader::compile_glsl_shader_with_second_chance(const shader_request_t* request) {
		while (compile_glsl_shader(request)) {
			printf("Try again (Y/n)? ");
			char response;
			scanf("%1c", &response);
			if (response == 'N' || response == 'n') {
				printf("\nGiving up.\n");
				return 1;
			}
			else
				printf("\nTrying again.\n");
		}
		return 0;
	}

	const char* Shader::get_shader_stage_name(VkShaderStageFlags stage) {
		switch (stage) {
		case VK_SHADER_STAGE_VERTEX_BIT: return "vert";
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return "tesc";
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return "tese";
		case VK_SHADER_STAGE_GEOMETRY_BIT: return "geom";
		case VK_SHADER_STAGE_FRAGMENT_BIT: return "frag";
		case VK_SHADER_STAGE_COMPUTE_BIT: return "comp";
		case VK_SHADER_STAGE_RAYGEN_BIT_KHR: return "rgen";
		case VK_SHADER_STAGE_INTERSECTION_BIT_KHR: return "rint";
		case VK_SHADER_STAGE_ANY_HIT_BIT_KHR: return "rahit";
		case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR: return "rchit";
		case VK_SHADER_STAGE_MISS_BIT_KHR: return "rmiss";
		case VK_SHADER_STAGE_CALLABLE_BIT_KHR: return "rcall";
		case VK_SHADER_STAGE_TASK_BIT_NV: return "task";
		case VK_SHADER_STAGE_MESH_BIT_NV: return "mesh";
		default: return NULL;
		};
	}

	void Shader::destroy_shader() {
		auto device = Graphics::Vulkan::getInstance()->GetDevice();
		if (shader->module != nullptr) vkDestroyShaderModule(device.device, shader->module, device.allocator);
		if(shader->spirv_code != nullptr) free(shader->spirv_code);
		shader->module = nullptr;
		shader->spirv_code = nullptr;
		// memset(shader, 0, sizeof(*shader));
	}

	std::map<ShaderType, unique_ptr<ShaderData>> shaderFactory;
	std::map<ShaderType, string> shaderFilePath;
	string dict = "../src/shaders/";

	void LoadShaderPaths()
	{
		shaderFilePath[ShaderType::Skybox] = dict+"Skybox";
		shaderFilePath[ShaderType::Outline] = dict+"Outline";
		shaderFilePath[ShaderType::Phone] = dict+"Phone";
		shaderFilePath[ShaderType::PBR] = dict+"Pbr_basic";
		shaderFilePath[ShaderType::BRDFLUT] = dict+"Brdf_lut";
		shaderFilePath[ShaderType::IRRADIANCE] = dict+"Irradiance_map";
		shaderFilePath[ShaderType::PREFILTER] = dict+"Prefilter_map";
		shaderFilePath[ShaderType::UI] = dict+"UIoverlay";
		shaderFilePath[ShaderType::GBUFFER] = dict+"Deferred_gbuffer";
		shaderFilePath[ShaderType::FULLSCREEN_VERT] = dict+"FullScreen";
		shaderFilePath[ShaderType::SSAO] = dict+"Ssao_generate";
		shaderFilePath[ShaderType::DEFAULT] = dict+"Default";
		shaderFilePath[ShaderType::PBR_Deferred] = dict+"Pbr_deferred";
		shaderFilePath[ShaderType::BLUR] = dict+"Blur";
		shaderFilePath[ShaderType::OMNISHADOW] = dict+"shadow/omni_shadow";
		shaderFilePath[ShaderType::COMPOSITION] = dict + "composition/Composition";
		shaderFilePath[ShaderType::CASCADE_SHADOW] = dict + "shadow/CascadeShadow";
	}

	map<string, unique_ptr<Shader> >shaderCollection;
	// std::vector< unique_ptr<Shader> > shaderCollection;

	VkPipelineShaderStageCreateInfo CreateShaderStage(ShaderType type, VkShaderStageFlagBits stage, vector<string>&& defs)
	{
		VkPipelineShaderStageCreateInfo info = {};
		string key = std::to_string(static_cast<int>(type)) + std::to_string(stage);
		for (auto str : defs) {
			key += str;
		}

		Shader* shader;
		if (!shaderCollection.count(key)) 
			shaderCollection[key] = make_unique<Shader>(shaderFilePath[type], "../src/shaders", "main", stage, std::forward<vector<string>>(defs));
		shader = shaderCollection[key].get();
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		info.stage = stage;
		info.module = shaderCollection[key]->GetShaderModule();
		info.pName = "main";

		return info;
	}

	void LoadShaders()
	{
		shaderFactory[ShaderType::Outline] = make_unique< ShaderData>("Outline");
		shaderFactory[ShaderType::Phone] = make_unique< ShaderData>("Phone");
		shaderFactory[ShaderType::BRDFLUT] = make_unique< ShaderData>("Brdf_lut");
		shaderFactory[ShaderType::UI] = make_unique< ShaderData>("UIoverlay");
		shaderFactory[ShaderType::FULLSCREEN_VERT] = make_unique< ShaderData>("FullScreen", false);
		shaderFactory[ShaderType::SSAO] = make_unique< ShaderData>(shaderFactory[ShaderType::FULLSCREEN_VERT]->vert_shader, "Ssao_generate");
	}
}