#ifndef MODEL_H
#define MODEL_H
#include "Vulkan.h"
#include <vector>
#include <glm/glm.hpp>
#include "Camera.h"
#include <map>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "VertexBuffer.h"
#include "Scene.h"

#include "Drawable/PhoneMaterial.h"
#include "Drawable/Texture.h"

#include "Bindable/IndexBuffer.h"
#include "Drawable/ModelWindowBase.h"


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

		std::shared_ptr<Graphics::Vulkan> vulkan_ptr;
	};

	class Mesh
	{
		friend class Model;
	public:
		Mesh(shared_ptr<Graphics::Vulkan> vulkan_ptr, const Dcb::VertexBuffer& vbuf, const std::vector<unsigned short>& ibuf);
		shared_ptr<Bind::VertexBuffer> vertex_buffer;
		shared_ptr<Bind::IndexBuffer> index_buffer;

	};

	class Node
	{
	public:
		Node(std::vector<Mesh*> meshPtrs, std::vector<MaterialBase*> matPtrs, const glm::mat4& transform, const char* name, int id) ;
		void AddChild(std::unique_ptr<Node> pChild);
		void Accept(ModelWindowBase* window);
		int GetId();
		string GetName();
		bool HasChild();
		glm::mat4& GetTransform();
		void SetTransform(glm::mat4 transform);
	private:
		vector<std::unique_ptr<Node>> childPtrs;
		vector< Mesh* > curMeshes;
		vector< MaterialBase* > curMats;
		string name;
		int id;
		glm::mat4 transform;
	};

	struct DrawItem
	{
		DrawItem(Mesh mesh, MaterialBase mat) : mesh(mesh), material(mat) {}
		Mesh mesh;
		MaterialBase material;
	};

	class Model : public ModelBase
	{
	public:

		Model(std::shared_ptr<Graphics::Vulkan> vulkan_ptr, shared_ptr<Control::Scene> scene_ptr, shared_ptr<Graphics::DescriptorPool> desc_pool,
			shared_ptr<Draw::Texture> texture_ptr,
			std::string file_path, std::string directory);

		void ParseMesh(const aiMesh& mesh, const aiMaterial* material);

		std::unique_ptr<Node> ParseNode(const aiNode& node, glm::mat4 nowTrans, int& nextId);

		void Update(int cur);

		void BuildDesc(shared_ptr<Graphics::DescriptorSetLayout> desc_layout_ptr);

		void Accept(ModelWindowBase* window);

		std::unique_ptr<Node> pRoot;
		
		vector<DrawItem> items;

		shared_ptr<Control::Scene> scene_ptr;
		shared_ptr<Graphics::DescriptorPool> desc_pool;
		shared_ptr<Draw::Texture> texture_ptr;
	private:
		std::string directory;
		int loadMaterialTextures(const aiMaterial* mat, aiTextureType type, string typeName);
	};
}



#endif // !MODEL_H
