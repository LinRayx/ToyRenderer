#include "Model.h"
#include <glm/gtc/matrix_transform.hpp>

#include "Exception/ModelException.h"
#include "DynamicVertex.h"

namespace Draw {

	glm::mat4 ModelBase::getModelMatrix()
	{
		glm::mat4 mat = glm::mat4(1.0);
		mat = glm::translate(mat, transform_s.translate);
		//mat = glm::rotate(mat, angle_in_degrees, glm::vec3(x, y, z));
		mat = glm::scale(mat, transform_s.scale);

		return mat;
	}

	Model::Model(std::shared_ptr<Graphics::Vulkan> vulkan_ptr, std::string file_path)
	{
		this->vulkan_ptr = vulkan_ptr;
		Assimp::Importer imp;
		const auto pScene = imp.ReadFile(file_path.c_str(),
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace
		);

		if (pScene == nullptr)
		{
			throw ModelException(__LINE__, __FILE__, imp.GetErrorString());
		}

		for (size_t i = 0; i < pScene->mNumMeshes; ++i) {
			ParseMesh(*pScene->mMeshes[i]);
		}

		pRoot = ParseNode(*pScene->mRootNode);
	}

	void Model::ParseMesh(const aiMesh& mesh)
	{
		using namespace Dcb;
		Dcb::VertexBuffer vbuf(
			std::move(
				Dcb::VertexLayout{}
				.Append(VertexLayout::Position2D)
				.Append(VertexLayout::Normal)
			)
		);

		for (uint32_t i = 0; i < mesh.mNumVertices; ++i) {
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

		Mesh mesh_s(vulkan_ptr, renderpass_ptr, cmdBuf_ptr, cmdQueue_ptr, scene_ptr->desc_ptr);
		mesh_s.Bind(BindType::VertexBuffer, make_shared<Bind::VertexBuffer>(vulkan_ptr, vbuf));
		mesh_s.Bind(BindType::VertexShader, make_shared<Bind::VertexShader>(vulkan_ptr, "../src/shaders/Phone.vert.glsl", "../src/shaders", "main"));
		mesh_s.Bind(BindType::PixelShader, make_shared<Bind::PixelShader>(vulkan_ptr, "../src/shaders/Phone.frag.glsl", "../src/shaders", "main"));

		meshes.emplace_back(std::move(mesh_s));
	}

	std::unique_ptr<Node> Model::ParseNode(const aiNode& node)
	{
		const glm::mat4* transform = reinterpret_cast<const glm::mat4* >(&node.mTransformation);
		
		std::vector<Mesh*> curMeshPtrs;
		curMeshPtrs.reserve(node.mNumMeshes);
		for (size_t i = 0; i < node.mNumMeshes; i++)
		{
			const auto meshIdx = node.mMeshes[i];
			curMeshPtrs.push_back(&(meshes[meshIdx]));
		}
		auto pNode = std::make_unique<Node>(std::move(curMeshPtrs), *transform);
		for (size_t i = 0; i < node.mNumChildren; i++)
		{
			pNode->AddChild(ParseNode(*node.mChildren[i]));
		}

		return pNode;
	}

	VkVertexInputBindingDescription Model::getBindingDescription()
	{
		return VkVertexInputBindingDescription();
	}

	std::vector<VkVertexInputAttributeDescription> Model::getAttributeDescriptions()
	{
		return std::vector<VkVertexInputAttributeDescription>();
	}

	void Model::Compile()
	{
		for (size_t i = 0; i < meshes.size(); ++i)
			meshes[i].Compile();
	}

	void Model::Update()
	{
	}

	size_t Model::getCount()
	{
		return size_t();
	}

	size_t Model::getSize()
	{
		return size_t();
	}

	void* Model::getData()
	{
		return nullptr;
	}

	Node::Node(std::vector<Mesh*> meshPtrs, const glm::mat4& transform)
		: curMeshes(std::move(meshPtrs))
	{
		this->transform = transform;
	}

	void Node::AddChild(std::unique_ptr<Node> pChild)
	{
		childPtrs.push_back(std::move(pChild));
	}

	Mesh::Mesh(shared_ptr<Graphics::Vulkan> vulkan_ptr, shared_ptr<Graphics::RenderPass> renderpass_ptr, shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr, shared_ptr<Graphics::CommandQueue> cmdQueue_ptr, shared_ptr<Graphics::DescriptorSetCore> desc_ptr)
	{
		draw_ptr = make_unique<Drawable>(vulkan_ptr);
		draw_ptr->Register<Draw::GraphicsType::DescriptorSet>(desc_ptr);

		draw_ptr->Register<Draw::GraphicsType::RenderPass>(renderpass_ptr);
		draw_ptr->Register<Draw::GraphicsType::CommandBuffer>(cmdBuf_ptr);
		draw_ptr->Register<Draw::GraphicsType::CommandQueue>(cmdQueue_ptr);
	}

	void Mesh::Bind(BindType type, std::shared_ptr<Bind::Bindable> elem)
	{
		draw_ptr->Register(type, elem);
	}

	void Mesh::Compile()
	{
		draw_ptr->CompilePipeline();
		draw_ptr->BuildCommandBuffer();
	}

}