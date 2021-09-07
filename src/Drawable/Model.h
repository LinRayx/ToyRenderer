#ifndef MODEL_H
#define MODEL_H
#include "VulkanCore/vulkan_core_headers.h"

#include <vector>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Drawable/Texture.h"
#include "Drawable/ModelWindowBase.h"
#include "Drawable/MaterialHeader.h"

#include "Utils/GloableClass.h"

namespace Draw
{

	class ModelBase
	{
	protected:
		struct Transform_S {
			glm::vec3 translate = glm::vec3(0, 0, 0);
			glm::vec3 rotate = glm::vec3(0, 0, 0);
			glm::vec3 scale = glm::vec3(1, 1, 1);
		} transform_s;

		glm::mat4 getModelMatrix();
	};

	class Mesh
	{
		friend class Model;
	public:
		Mesh(const aiMesh& mesh, const aiMaterial* material, string directoy);
		shared_ptr<Bind::VertexBuffer> vertex_buffer;
		shared_ptr<Bind::IndexBuffer> index_buffer;
		void SetMaterial(MaterialBase* mat);
		void SetTransform(glm::mat4 trans);
		void SetPosition(glm::vec3 pos);
		void SetScale(glm::vec3 scale);
		MaterialBase* GetMaterial();
		glm::mat4 GetTransform();
	private:
		const aiMaterial* material;
		vector<MaterialBase*> mat_ptrs;
		string dire;
		string name;
		glm::mat4 transform;
	};

	class Node
	{
	public:
		Node(std::vector<Mesh*> meshPtrs, const glm::mat4& transform, const char* name, int id) ;
		void AddChild(std::unique_ptr<Node> pChild);
		void Accept(ModelWindowBase* window);
		int GetId();
		string GetName();
		bool HasChild();
		bool AddMaterialUI();
		glm::mat4& GetTransform();
		void SetTransform(glm::mat4 transform);
		void Traverse(vector<Mesh*> meshes);

	private:
		vector<std::unique_ptr<Node>> childPtrs;
		vector< Mesh* > curMeshes;
		string name;
		int id;
		glm::mat4 transform;
	};

	class Model : public ModelBase
	{
	public:

		Model(std::string file_path, std::string directory);

		void ParseMesh(const aiMesh& mesh, const aiMaterial* material);

		std::unique_ptr<Node> ParseNode(const aiNode& node, glm::mat4 nowTrans, int& nextId);

		void Update(int cur);

		void Accept(ModelWindowBase* window);

		void AddMaterial(MaterialType type, glm::vec4 color = glm::vec4(1, 1, 1, 1));
		void AddMaterial(DefaultMaterial* mat);

		void Compile();

		void BuildCommandBuffer(Draw::MaterialType matType, shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr);

		std::unique_ptr<Node> pRoot;
		
		vector<Mesh> meshes;

		struct Object
		{
			Object(Mesh mesh) : mesh(std::move(mesh)) {}
			Mesh mesh;
			std::map<MaterialType, MaterialBase*> materials;
		};
		vector<Object> objects;

		string name;
	private:
		Assimp::Importer imp;
		std::string directory;
	};
}



#endif // !MODEL_H
