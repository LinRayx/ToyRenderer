#include "mesh.h"
#include <string.h>
#include <iostream>


namespace Mesh {
	int create_mesh_buffer(mesh_buffer_t* buffer, uint32_t size, void* data, const device_t* device) {
		memset(buffer, 0, sizeof(*buffer));
		VkBufferCreateInfo bufferInfo = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = size,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};

		if (create_buffers(&buffer->vertices, device, &bufferInfo, 1, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			return 1;
		}
		if (vkMapMemory(device->device, buffer->vertices.memory, 0, buffer->vertices.size, 0, &buffer->data)) {
			return 1;
		}
		memcpy(buffer->data, data, (size_t)buffer->vertices.size);
	}

	int create_vertices_buffer (vertices_buffer_t* buffer, uint32_t size, void* data, const device_t* device) {
		memset(buffer, 0, sizeof(*buffer));
		VkBufferCreateInfo bufferInfo = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = size,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};

		if (create_buffers(&buffer->buffers, device, &bufferInfo, 1, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			return 1;
		}
		if (vkMapMemory(device->device, buffer->buffers.memory, 0, buffer->buffers.size, 0, &buffer->data)) {
			return 1;
		}
		memcpy(buffer->data, data, (size_t)buffer->buffers.size);
	}

	int create_indices_buffer(indices_buffer_t* buffer, uint32_t size, void* data, const device_t* device) {
		memset(buffer, 0, sizeof(*buffer));
		VkBufferCreateInfo bufferInfo = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = size,
			.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};

		if (create_buffers(&buffer->buffers, device, &bufferInfo, 1, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			return 1;
		}
		if (vkMapMemory(device->device, buffer->buffers.memory, 0, buffer->buffers.size, 0, &buffer->data)) {
			return 1;
		}
		memcpy(buffer->data, data, (size_t)buffer->buffers.size);
	}



	void Model::LoadModel(const device_t* device, std::string fileName) {
		Assimp::Importer imp;
		const auto pScene = imp.ReadFile(fileName.c_str(),
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GenNormals);

		if (pScene == nullptr) {
			std::cout << "pScene is nullptr" << std::endl;
		}

		for (size_t i = 0; i < pScene->mNumMeshes; ++i) {
			meshPtrs.push_back(ParseMesh(device, *pScene->mMeshes[i]));
		}
	}

	std::unique_ptr<Mesh> Model::ParseMesh(const device_t* device, const aiMesh& mesh)
	{
		using Vertex::VertexLayout;
		Vertex::VertexBuffer vbuf(std::move(
			VertexLayout{}
			.Append(VertexLayout::Position3D)
			.Append(VertexLayout::Normal)
		));

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			vbuf.EmplaceBack(
				*reinterpret_cast<glm::vec3*>(&mesh.mVertices[i]),
				*reinterpret_cast<glm::vec3*>(&mesh.mNormals[i])
			);
		}

		std::vector<unsigned short> indices;
		indices.reserve(mesh.mNumFaces * 3);
		for (unsigned int i = 0; i < mesh.mNumFaces; i++)
		{
			const auto& face = mesh.mFaces[i];
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}

		vertices_buffer_t vertices_b;
		indices_buffer_t indices_b;
		indices_b.size = indices.size();
		create_vertices_buffer(&vertices_b, vbuf.SizeBytes(), (void*)vbuf.GetData(), device);
		create_indices_buffer(&indices_b, indices.size() * sizeof(unsigned short), indices.data(), device);
		
		return std::make_unique<Mesh>(device, std::move(vertices_b), std::move(indices_b), std::move(vbuf.GetLayout().GetBindingDescription()), std::move(vbuf.GetLayout().GetAttributeDescriptions()));
	}

	
	void Mesh::Draw(VkCommandBuffer* cmd, VkPipeline* pipeline)
	{
		const VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(*cmd, 0, 1, &vertices.buffers.buffers->buffer, offsets);


		vkCmdBindIndexBuffer(*cmd, indices.buffers.buffers->buffer, 0, VK_INDEX_TYPE_UINT16);

		vkCmdDrawIndexed(*cmd, indices.size, 1, 0, 0, 0);
	}

	Mesh::~Mesh()
	{
		if (device == nullptr) return;
		destroy_buffers(&vertices.buffers, device);
		destroy_buffers(&indices.buffers, device);
	}

}