#pragma once

#ifndef MESH_H
#define MESH_H


#include "vulkan_basics.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <memory>

#include "vertex.h"

namespace Mesh {
	
	struct mesh_buffer_t {
		buffers_s vertices;
		void* data;
	};

	struct vertices_buffer_t {
		buffers_s buffers;
		void* data;
	};

	struct indices_buffer_t {
		buffers_s buffers;
		void* data;
		uint32_t size;
	};

	int create_mesh_buffer(mesh_buffer_t* buffer, uint32_t size, void* data, const device_t* device);
	int create_vertices_buffer(vertices_buffer_t* buffer, uint32_t size, void* data, const device_t* device);
	int create_indices_buffer(indices_buffer_t* buffer, uint32_t size, void* data, const device_t* device);
	class Mesh
	{
	public:

		Mesh(vertices_buffer_t vertices, indices_buffer_t indices, VkVertexInputBindingDescription bindingDescription, std::vector<VkVertexInputAttributeDescription> attributeDescriptions)
		{
			this->vertices = vertices;
			this->indices = indices;
			this->bindingDescription = bindingDescription;
			this->attributeDescriptions = attributeDescriptions;
		}

		void Draw(VkCommandBuffer* cmd, VkPipeline* pipeline);

	private:
		
		vertices_buffer_t vertices;
		indices_buffer_t indices;
		VkVertexInputBindingDescription bindingDescription;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	};

	class Model {
	public:
		Model(const device_t* device, std::string fileName);
	private:
		static std::unique_ptr<Mesh> ParseMesh(const device_t* device, const aiMesh& mesh);
		//std::unique_ptr<Node> ParseNode(int& nextId, const aiNode& node) noexcept;
	private:
		std::vector<std::unique_ptr<Mesh>> meshPtrs;
	};

}

#endif // !MESH_H
