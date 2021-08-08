#pragma once
#include "Shader.h"
#include "string_utilities.h"

namespace Bind
{
	void Shader::CompileShader(std::string shader_file_path, std::string include_path, std::string entry_point, VkShaderStageFlagBits stage, 
		shader_t* shader)
	{
		shader_request_t shader_request = {};
		shader_request.shader_file_path = shader_file_path.c_str();
		shader_request.include_path = include_path.c_str();
		shader_request.entry_point = entry_point.c_str();
		shader_request.stage = stage;

		if (compile_glsl_shader_with_second_chance(shader, &shader_request)) {
			exit(1);
		}

	}
	int Shader::compile_glsl_shader(shader_t* shader, const shader_request_t* request) {
		auto device = vulkan_ptr->GetDevice();
		if (!get_shader_stage_name(request->stage)) {
			printf("Invalid stage specification %u passed for shader %s.", request->stage, request->shader_file_path);
			return 1;
		}
		//char buf[80];
		//getcwd(buf, sizeof(buf));
		//printf("current working directory: %s\n", buf);

		// Verify that the shader file exists by opening and closing it
#ifndef NDEBUG
		FILE* shader_file = fopen(request->shader_file_path, "r");
		if (!shader_file) {
			printf("The shader file at path %s does not exist or cannot be opened.\n", request->shader_file_path);
			return 1;
		}
		fclose(shader_file);
#endif
		// Delete the prospective output file such that we can verify its existence
		// to see if the compiler did anything
		const char* spirv_path_pieces[] = { request->shader_file_path, ".spv" };
		char* spirv_path = concatenate_strings(COUNT_OF(spirv_path_pieces), spirv_path_pieces);
		remove(spirv_path);
		// Build the part of the command line for defines
		const char** define_pieces = (const char**)malloc(sizeof(char*) * 2 * request->define_count);
		for (uint32_t i = 0; i != request->define_count; ++i) {
			define_pieces[2 * i + 0] = " -D";
			define_pieces[2 * i + 1] = request->defines[i];
		}
		char* concatenated_defines = concatenate_strings(2 * request->define_count, define_pieces);
		free(define_pieces);
		// Construct the command line
		const char* command_line_pieces[] = {
			"glslangValidator -V100 --target-env spirv1.5 ",
			"-S ", get_shader_stage_name(request->stage),
	#ifndef NDEBUG
			" -g -Od ",
	#endif
			concatenated_defines,
			" -I\"", request->include_path, "\" ",
			"--entry-point ", request->entry_point,
			" -o \"", spirv_path,
			"\" \"", request->shader_file_path, "\""
		};
		char* command_line = concatenate_strings(COUNT_OF(command_line_pieces), command_line_pieces);
		free(concatenated_defines);
		// Check whether command processing is available at all
#ifndef NDEBUG
		if (!system(NULL)) {
			printf("No command processor is available. Cannot invoke the compiler to compile the shader at path %s.\n", request->shader_file_path);
			free(command_line);
			free(spirv_path);
			return 1;
		}
#endif
		// Invoke the command line and see whether it produced an output file
		system(command_line);
		FILE* file = fopen(spirv_path, "rb");
		if (!file) {
			printf("glslangValidator failed to compile the shader at path %s. The full command line is:\n%s\n", request->shader_file_path, command_line);
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
			printf("Failed to create a shader module from %s.\n", request->shader_file_path);
			destroy_shader(shader);
		}
		return 0;
	}


	int Shader::compile_glsl_shader_with_second_chance(shader_t* shader, const shader_request_t* request) {
		while (compile_glsl_shader(shader, request)) {
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

	void Shader::destroy_shader(shader_t* shader) {
		auto device = vulkan_ptr->GetDevice();
		if (shader->module) vkDestroyShaderModule(device.device, shader->module, device.allocator);
		free(shader->spirv_code);
		memset(shader, 0, sizeof(*shader));
	}


}