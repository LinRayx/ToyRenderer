#include "Model.h"
#include <glm/gtc/matrix_transform.hpp>

#include "Exception/ModelException.h"
#include "DynamicVertex.h"
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Draw {

	glm::mat4 ModelBase::getModelMatrix()
	{
		glm::mat4 mat = glm::mat4(1.0);
		mat = glm::translate(mat, transform_s.translate);
		//mat = glm::rotate(mat, angle_in_degrees, glm::vec3(x, y, z));
		mat = glm::scale(mat, transform_s.scale);

		return mat;
	}

	Model::Model(shared_ptr<Control::Scene> scene_ptr,
		std::string file_path, std::string directory)
	{
		this->directory = directory;
		this->scene_ptr = scene_ptr;
		
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
		Mesh t_mesh(mesh, material, directory);

		objects.emplace_back(Object(std::move(t_mesh)));
	}

	std::unique_ptr<Node> Model::ParseNode(const aiNode& node, glm::mat4 nowTrans, int& nextId)
	{
		const glm::mat4* tmp = reinterpret_cast<const glm::mat4* >(&node.mTransformation);
		// nowTrans *= *tmp;
		std::vector<Mesh*> curMeshPtrs;
		curMeshPtrs.reserve(node.mNumMeshes);
		std::vector<MaterialBase*> curMatPtrs;
		curMatPtrs.reserve(node.mNumMeshes);
		for (size_t i = 0; i < node.mNumMeshes; i++)
		{
			const auto meshIdx = node.mMeshes[i];
			objects[meshIdx].mesh.SetTransform(nowTrans);
			curMeshPtrs.push_back(&objects[meshIdx].mesh);
		}
		auto pNode = std::make_unique<Node>(std::move(curMeshPtrs), std::move(curMatPtrs), *tmp, node.mName.C_Str(), nextId);
		for (size_t i = 0; i < node.mNumChildren; i++)
		{
			pNode->AddChild(ParseNode(*node.mChildren[i], nowTrans, ++nextId));
		}

		return pNode;
	}

	void Model::Update(int cur)
	{
		for (auto& obj : objects) {
			for (auto& mat : obj.materials) {
				scene_ptr->Update(mat.second);
				mat.second->Update(cur);
			}
		}
	}

	void Model::BuildDesc(shared_ptr<Graphics::DescriptorSetLayout> desc_layout_ptr, MaterialType matType)
	{

	}

	void Model::Accept(ModelWindowBase* window)
	{
		pRoot->Accept(window);
	}

	void Model::AddMaterial(MaterialType type)
	{
		for (auto& it : objects) {
			MaterialBase* material = nullptr;
			switch (type)
			{
			case Draw::MaterialType::Phone:
				material = new PhoneMaterial;
				break;
			case Draw::MaterialType::Outline:
				material = new OutlineMaterial;
				break;
			case Draw::MaterialType::Skybox:
				material = new SkyboxMaterial;
				break;
			case Draw::MaterialType::PBR:
				material = new PBRMaterial;
				break;
			default:
				throw std::runtime_error("can not find suitable material!");
			}

			it.mesh.SetMaterial(material);
			material->BindMeshData(it.mesh.vertex_buffer, it.mesh.index_buffer);
			material->SetValue("Model", "modelTrans", it.mesh.GetTransform());
			it.materials[type] = material;
		}
	}

	void Model::Compile()
	{
		for (auto& obj : objects) {
			for (auto& mat : obj.materials) {
				mat.second->Compile();
			}
		}
	}

	void Model::BuildCommandBuffer(Draw::MaterialType matType, shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr)
	{
		for (auto& obj : objects) {
			if (obj.materials.count(matType)) {
				obj.materials[matType]->BuildCommandBuffer(cmdBuf_ptr);
			}
		}
	}

	Node::Node(std::vector<Mesh*> meshPtrs, std::vector<MaterialBase*> matPtrs,
		const glm::mat4& transform, const char* name, int id)
		: curMeshes(std::move(meshPtrs)), curMats(std::move(matPtrs)), id(id)
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

	glm::mat4& Node::GetTransform()
	{
		return transform;
	}

	void Node::SetTransform(glm::mat4 transform)
	{
		this->transform *= transform;
		for (auto& mat : curMats) {
			mat->SetValue("Model", "modelTrans", transform);
		}
		for (auto& child : childPtrs) {
			child->SetTransform(transform);
		}
		// material.SetValue("Model", "modelTrans", nowTrans);
	}

	void Node::Traverse(vector<Mesh*> meshes)
	{
		
	}

	Mesh::Mesh(const aiMesh& mesh, const aiMaterial* material, string directoy)
	{
		dire = directoy;
		this->material = material;
		this->name = mesh.mName.C_Str();

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
			vbuf.EmplaceBack(
				*reinterpret_cast<glm::vec3*>(&(mesh.mVertices[i])),
				*reinterpret_cast<glm::vec3*>(&(mesh.mNormals[i])),
				*reinterpret_cast<glm::vec2*>(&mesh.mTextureCoords[0][i])
			);
		}

		std::vector<unsigned short> ibuf;
		ibuf.reserve(mesh.mNumFaces * 3);
		for (unsigned int i = 0; i < mesh.mNumFaces; i++)
		{
			const auto& face = mesh.mFaces[i];
			assert(face.mNumIndices == 3);
			ibuf.push_back(face.mIndices[0]);
			ibuf.push_back(face.mIndices[1]);
			ibuf.push_back(face.mIndices[2]);
		}

		vertex_buffer = make_shared<Bind::VertexBuffer>(vbuf);
		index_buffer = make_shared<Bind::IndexBuffer>(ibuf);


	}
	void Mesh::SetMaterial(MaterialBase* mat)
	{
		mat->LoadModelTexture(material, dire, name);
	}
	void Mesh::SetTransform(glm::mat4 trans)
	{
		this->transform = trans;
	}
	glm::mat4 Mesh::GetTransform()
	{
		return transform;
	}
}