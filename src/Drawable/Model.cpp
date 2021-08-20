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

	Model::Model(std::shared_ptr<Graphics::Vulkan> vulkan_ptr, shared_ptr<Control::Scene> scene_ptr, shared_ptr<Graphics::DescriptorPool> desc_pool,
		std::string file_path)
	{
		this->vulkan_ptr = vulkan_ptr;
		this->scene_ptr = scene_ptr;
		this->desc_pool = desc_pool;
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

		pRoot = ParseNode(*pScene->mRootNode, glm::mat4(1));
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
		Mesh t_mesh(vulkan_ptr, vbuf);
		Material mat(vulkan_ptr, desc_pool);
		scene_ptr->InitSceneData(&mat);
		
		Dcb::RawLayout transBuf;
		transBuf.Add<Dcb::Matrix>("modelTrans");
		mat.AddLayout("Model", std::move(transBuf), Graphics::LayoutType::MODEL, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::VERTEX);

		items.emplace_back(DrawItem( std::move(t_mesh), std::move(mat) ));
	}

	std::unique_ptr<Node> Model::ParseNode(const aiNode& node, glm::mat4 nowTrans)
	{
		const glm::mat4* tmp = reinterpret_cast<const glm::mat4* >(&node.mTransformation);
		
		
		nowTrans = (*tmp) * nowTrans;

		std::vector<Mesh*> curMeshPtrs;
		curMeshPtrs.reserve(node.mNumMeshes);

		for (size_t i = 0; i < node.mNumMeshes; i++)
		{
			const auto meshIdx = node.mMeshes[i];
			items[meshIdx].material.Update("Model", "modelTrans", nowTrans);

		}
		auto pNode = std::make_unique<Node>(std::move(curMeshPtrs), *tmp);
		for (size_t i = 0; i < node.mNumChildren; i++)
		{
			pNode->AddChild(ParseNode(*node.mChildren[i], nowTrans));
		}

		return pNode;
	}

	void Model::Update(int cur)
	{
		for (size_t i = 0; i < items.size(); ++i) {
			scene_ptr->Update(&items[i].material);
		}
	}

	void Model::BuildDesc(shared_ptr<Graphics::DescriptorSetLayout> desc_layout_ptr)
	{
		for (auto it : items) {
			it.material.Compile(desc_layout_ptr);
		}
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

	Mesh::Mesh(shared_ptr<Graphics::Vulkan> vulkan_ptr, const Dcb::VertexBuffer& vbuf)
	{
		vertex_buffer = make_shared<Bind::VertexBuffer>(vulkan_ptr, vbuf);
	}
}