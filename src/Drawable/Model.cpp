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
		shared_ptr<Draw::Texture> texture_ptr,
		std::string file_path, std::string directory)
	{
		this->directory = directory;
		this->vulkan_ptr = vulkan_ptr;
		this->scene_ptr = scene_ptr;
		this->desc_pool = desc_pool;
		this->texture_ptr = texture_ptr;

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
			ParseMesh(*pScene->mMeshes[i], pScene->mMaterials[pScene->mMeshes[i]->mMaterialIndex]);
		}

		auto t = glm::mat4(1);
		t[1][1] = -1;
		int nextId = 0;
		pRoot = ParseNode(*pScene->mRootNode, t, nextId);
	}

	void Model::ParseMesh(const aiMesh& mesh, const aiMaterial* material)
	{
		using namespace Dcb;
		Dcb::VertexBuffer vbuf(
			std::move(
				Dcb::VertexLayout{}
				.Append(VertexLayout::Position3D)
				.Append(VertexLayout::Normal)
				.Append(VertexLayout::Texture2D)
			)
		);


		for (uint32_t i = 0; i < mesh.mNumVertices; ++i) {
			//glm::vec3 tt = *reinterpret_cast<glm::vec3*>(&(mesh.mVertices[i]));
			//std::cout << tt.x << " " << tt.y << " " << tt.z << std::endl;
			vbuf.EmplaceBack(
				*reinterpret_cast<glm::vec3*>(&(mesh.mVertices[i])),
				*reinterpret_cast<glm::vec3*>(&(mesh.mNormals[i])),
				*reinterpret_cast<glm::vec2*>(&mesh.mTextureCoords[0][i])
			);
			//std::cout << mesh.mVertices[i].x <<" " << mesh.mVertices[i].y << " " << mesh.mVertices[i].z << std::endl;
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

		int cnt = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		
		Mesh t_mesh(vulkan_ptr, vbuf, indices);
		Material mat(vulkan_ptr, desc_pool);
		scene_ptr->InitSceneData(&mat);
		
		Dcb::RawLayout transBuf;
		transBuf.Add<Dcb::Matrix>("modelTrans");
		mat.AddLayout("Model", std::move(transBuf), Graphics::LayoutType::MODEL, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::VERTEX);
		
		for (int i = 0; i < cnt; ++i) {
			string key = "texture_diffuse_" + to_string(i);
			mat.AddTexture(Graphics::LayoutType::MODEL, Graphics::StageFlag::FRAGMENT, texture_ptr->nameToTex[key].textureImageView, texture_ptr->nameToTex[key].textureSampler);
		}

		items.emplace_back(DrawItem( std::move(t_mesh), std::move(mat) ));
	}

	std::unique_ptr<Node> Model::ParseNode(const aiNode& node, glm::mat4 nowTrans, int& nextId)
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
		auto pNode = std::make_unique<Node>(std::move(curMeshPtrs), *tmp, node.mName.C_Str(), nextId);
		for (size_t i = 0; i < node.mNumChildren; i++)
		{
			pNode->AddChild(ParseNode(*node.mChildren[i], nowTrans, ++nextId));
		}

		return pNode;
	}

	void Model::Update(int cur)
	{
		for (size_t i = 0; i < items.size(); ++i) {
			scene_ptr->Update(&items[i].material);
			items[i].material.Update(cur);
		}
		
	}

	void Model::BuildDesc(shared_ptr<Graphics::DescriptorSetLayout> desc_layout_ptr)
	{
		for (auto it : items) {
			it.material.Compile(desc_layout_ptr);
		}
	}

	void Model::Accept(ModelWindowBase* window)
	{
		pRoot->Accept(window);
	}

	int Model::loadMaterialTextures(const aiMaterial* mat, aiTextureType type, string typeName)
	{
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);

			texture_ptr->CreateTexture(directory + str.C_Str(), typeName + "_" + to_string(i));
		}

		return mat->GetTextureCount(type);
	}


	Node::Node(std::vector<Mesh*> meshPtrs, const glm::mat4& transform, const char* name, int id)
		: curMeshes(std::move(meshPtrs)), id(id)
	{
		this->transform = transform;
		this->name = name;
	}

	void Node::AddChild(std::unique_ptr<Node> pChild)
	{
		childPtrs.push_back(std::move(pChild));
	}

	void Node::Accept(ModelWindowBase* window)
	{
		if (window->PushNode(*this)) {
			for (auto& it : childPtrs) {
				it->Accept(window);
			}
			window->PopNode(*this);
		}
	}

	int Node::GetId()
	{
		return id;
	}

	string Node::GetName()
	{
		return name;
	}

	bool Node::HasChild()
	{
		return childPtrs.size() > 0;
	}

	Mesh::Mesh(shared_ptr<Graphics::Vulkan> vulkan_ptr, const Dcb::VertexBuffer& vbuf, const std::vector<unsigned short>& ibuf)
	{
		vertex_buffer = make_shared<Bind::VertexBuffer>(vulkan_ptr, vbuf);
		index_buffer = make_shared<Bind::IndexBuffer>(vulkan_ptr, ibuf);
	}
}