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
#include "Drawable.h"
#include "Scene.h"
#include "Drawable/Material.h"

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
		Mesh(shared_ptr<Graphics::Vulkan> vulkan_ptr, const Dcb::VertexBuffer& vbuf);
		shared_ptr<Bind::VertexBuffer> vertex_buffer;

	};

	class Node
	{
	public:
		Node(std::vector<Mesh*> meshPtrs, const glm::mat4& transform) ;
		void AddChild(std::unique_ptr<Node> pChild) ;
	private:
		vector<std::unique_ptr<Node>> childPtrs;
		vector< Mesh* > curMeshes;
		glm::mat4 transform;
	};

	struct DrawItem
	{
		DrawItem(Mesh mesh, Material mat) : mesh(mesh), material(mat) {}
		Mesh mesh;
		Material material;
	};

	class Model : public ModelBase
	{
	public:

		Model(std::shared_ptr<Graphics::Vulkan> vulkan_ptr, shared_ptr<Control::Scene> scene_ptr, shared_ptr<Graphics::DescriptorPool> desc_pool,
			std::string file_path);

		void ParseMesh(const aiMesh& mesh);

		std::unique_ptr<Node> ParseNode(const aiNode& node, glm::mat4 nowTrans);

		void Update(int cur);

		void BuildDesc(shared_ptr<Graphics::DescriptorSetLayout> desc_layout_ptr);

		void Register(shared_ptr<Control::Scene> scene_ptr)
		{
			this->scene_ptr = scene_ptr;
		}

		std::unique_ptr<Node> pRoot;
		
		vector<DrawItem> items;

		shared_ptr<Control::Scene> scene_ptr;
		shared_ptr<Graphics::DescriptorPool> desc_pool;

	};
}



#endif // !MODEL_H
