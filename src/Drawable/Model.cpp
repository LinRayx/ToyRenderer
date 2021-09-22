#include "Model.h"
#include <glm/gtc/matrix_transform.hpp>

#include "Exception/ModelException.h"
#include "DynamicVertex.h"
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Draw {

	map<string, shared_ptr<Bind::VertexBuffer> > nameToVertex_buffer;
	map<string, shared_ptr<Bind::IndexBuffer> > nameToIndex_buffer;

	glm::mat4 ModelBase::getModelMatrix()
	{
		glm::mat4 mat = glm::mat4(1.0);
		mat = glm::translate(mat, transform_s.translate);
		//mat = glm::rotate(mat, angle_in_degrees, glm::vec3(x, y, z));
		mat = glm::scale(mat, transform_s.scale);

		return mat;
	}

	Model::Model(
		std::string file_path, std::string directory, glm::mat4 translate, glm::mat4 rotate, glm::mat4 scale)
	{
		this->directory = directory;
		
		const auto pScene = imp.ReadFile(file_path.c_str(),
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace
		);
		size_t offset = file_path.find_last_of('/');
		size_t len = file_path.length();
		name = file_path.substr(offset+1, len - offset);

		if (pScene == nullptr)
		{
			throw ModelException(__LINE__, __FILE__, imp.GetErrorString());
		}

		for (size_t i = 0; i < pScene->mNumMeshes; ++i) {
			ParseMesh(file_path, *pScene->mMeshes[i], pScene->mMaterials[pScene->mMeshes[i]->mMaterialIndex]);
		}

		int nextId = 0;
		pRoot = ParseNode(*pScene->mRootNode, translate, rotate, scale, nextId);
	}

	Model::~Model()
	{
		for (auto it : objects) {
			for (auto& mat : it.materials) {
				delete mat.second;
			}
		}
	}

	void Model::ParseMesh(string file_path, const aiMesh& mesh, const aiMaterial* material)
	{
		Mesh t_mesh(file_path, mesh, material, directory);

		objects.emplace_back(Object(std::move(t_mesh)));
	}

	std::unique_ptr<Node> Model::ParseNode(const aiNode& node, glm::mat4 translate, glm::mat4 rotate, glm::mat4 scale, int& nextId)
	{
		// const glm::mat4* tmp = reinterpret_cast<const glm::mat4* >(&node.mTransformation);
		// nowTrans *= *tmp;
		std::vector<Mesh*> curMeshPtrs;
		curMeshPtrs.reserve(node.mNumMeshes);

		for (size_t i = 0; i < node.mNumMeshes; i++)
		{
			const auto meshIdx = node.mMeshes[i];
			objects[meshIdx].mesh.SetTransform(translate, rotate, scale);
			curMeshPtrs.push_back(&objects[meshIdx].mesh);
		}
		auto pNode = std::make_unique<Node>(std::move(curMeshPtrs), rotate, translate, scale, node.mName.C_Str(), nextId);
		for (size_t i = 0; i < node.mNumChildren; i++)
		{
			pNode->AddChild(ParseNode(*node.mChildren[i], rotate , translate, scale, ++nextId));
		}

		return pNode;
	}

	void Model::Update(int cur)
	{
		for (auto& obj : objects) {
			for (auto& mat : obj.materials) {
				mat.second->UpdateSceneData();
				mat.second->Update(cur);
			}
		}
	}

	void Model::Accept(ModelWindowBase* window)
	{
		pRoot->Accept(window);
	}

	void Model::AddMaterial(MaterialType type, glm::vec4 color)
	{
		// ÄÚ´æÐ¹Â©
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
			case Draw::MaterialType::GBuffer:
				material = new DeferredMaterial;
				break;
			case Draw::MaterialType::DEFAULT:
				material = new DefaultMaterial(color);
				break;
			case Draw::MaterialType::OMNISHADOW:
				material = new OnmiShadowGenMaterial;
				break;
			case Draw::MaterialType::CASCADESHADOW:
				material = new CascadeShadowMaterial;
				break;
			default:
				throw std::runtime_error("can not find suitable material!");
			}
			material->SetModelName(this->name);
			it.mesh.SetMaterial(material);
			material->BindMeshData(it.mesh.vertex_buffer, it.mesh.index_buffer);
			material->SetTransform(it.mesh.GetTranslate(), it.mesh.GetRotate(), it.mesh.GetScale());
			// material->SetValue("Model", "modelTrans", it.mesh.GetTransform());
			it.materials[type] = material;
		}
	}

	void Model::AddMaterial(DefaultMaterial* mat)
	{
		for (auto& it : objects) {
			auto type = mat->GetMaterailType();
			mat->SetModelName(this->name);
			it.mesh.SetMaterial(mat);
			it.mesh.SetPosition(mat->GetPosition());
			mat->BindMeshData(it.mesh.vertex_buffer, it.mesh.index_buffer);
			mat->SetTransform(it.mesh.GetTranslate(), it.mesh.GetRotate(), it.mesh.GetScale());
			// mat->SetValue("Model", "modelTrans", it.mesh.GetTransform());
			it.materials[type] = mat;
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

	void Model::BuildCommandBuffer(Draw::MaterialType matType, shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr, int index, int face)
	{
		for (auto& obj : objects) {
			if (obj.materials.count(matType)) {
				obj.materials[matType]->BuildCommandBuffer(cmdBuf_ptr, index, face);
			}
		}
	}

	Node::Node(std::vector<Mesh*> meshPtrs,
		glm::mat4 translate, glm::mat4 rotate, glm::mat4 scale,
		const char* name, int id)
		: curMeshes(std::move(meshPtrs)),id(id)
	{
		this->translate = translate;
		this->rotate = rotate;
		this->scale = scale;
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

	bool Node::AddMaterialUI()
	{
		bool dirty = false;
		for (auto& mesh : curMeshes) {
			auto mat = mesh->GetMaterial();
			dirty |= mat->SetUI();
		}
		return dirty;
	}

	glm::mat4& Node::GetTranslate()
	{
		return translate;
	}

	glm::mat4& Node::GetRotate()
	{
		return rotate;
	}

	void Node::SetTransform(glm::mat4 translate, glm::mat4 rotate, glm::mat4 scale)
	{
		this->translate *= translate;
		this->rotate *= rotate;
		for (auto& mesh : curMeshes) {
			mesh->SetTransform(translate, rotate, scale);
		}
		for (auto& child : childPtrs) {
			child->SetTransform(translate, rotate, scale);
		}
	}

	void Node::Traverse(vector<Mesh*> meshes)
	{
		
	}


	Mesh::Mesh(string file_path, const aiMesh& mesh, const aiMaterial* material, string directoy)
	{
		dire = directoy;
		this->material = material;
		this->name = mesh.mName.C_Str();

		if (nameToIndex_buffer.count(file_path)) {
			vertex_buffer = nameToVertex_buffer[file_path];
			index_buffer = nameToIndex_buffer[file_path];
		}
		else {
			using namespace Dcb;
			Dcb::VertexBuffer vbuf(
				std::move(
					Dcb::VertexLayout{}
					.Append(VertexLayout::Position3D)
					.Append(VertexLayout::Normal)
					.Append(VertexLayout::Texture2D)
					.Append(VertexLayout::Tangent)
					//.Append(VertexLayout::Float3Color)
					//.Append(VertexLayout::Float3Color)
					//.Append(VertexLayout::Float3Color)
				)
			);

			for (uint32_t i = 0; i < mesh.mNumVertices; ++i) {
				if (Gloable::FilpY) {
					mesh.mVertices[i].y *= -1;
					mesh.mNormals[i].y *= -1;
				}

				vbuf.EmplaceBack(
					*reinterpret_cast<glm::vec3*>(&(mesh.mVertices[i])),
					*reinterpret_cast<glm::vec3*>(&(mesh.mNormals[i])),
					*reinterpret_cast<glm::vec2*>(&mesh.mTextureCoords[0][i]),
					*reinterpret_cast<glm::vec4*>(&mesh.mTangents[i])
					//Gloable::GetPreComputeLT(i, 0),
					//Gloable::GetPreComputeLT(i, 1),
					//Gloable::GetPreComputeLT(i, 2)
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

			nameToIndex_buffer[file_path] = index_buffer;
			nameToVertex_buffer[file_path] = vertex_buffer;
		}
	}
	void Mesh::SetMaterial(MaterialBase* mat)
	{
		mat->LoadModelTexture(material, dire, name);
		mat_ptrs.push_back(mat);
	}
	void Mesh::SetTransform(glm::mat4 translate, glm::mat4 rotate, glm::mat4 scale)
	{
		this->translate = translate;
		this->rotate = rotate;
		this->scale = scale;
		for (auto& ptr : mat_ptrs) {
			ptr->SetTransform(translate, rotate, scale);
		}
	}

	void Mesh::SetPosition(glm::vec3 pos)
	{
		translate = glm::translate(glm::mat4(1.0f), pos);
	}

	MaterialBase* Mesh::GetMaterial()
	{
		return mat_ptrs[0];
	}

	glm::mat4 Mesh::GetTranslate()
	{
		return translate;
	}

	glm::mat4 Mesh::GetRotate()
	{
		return rotate;
	}
	glm::mat4 Mesh::GetScale()
	{
		return scale;
	}
}