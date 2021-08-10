#ifndef MODEL_H
#define MODEL_H
#include "Vulkan.h"
#include <vector>
#include "DescriptorSet.h"
#include "DynamicConstant.h"
#include "DescriptorPool.h"
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
namespace Draw
{
	class ModelBase
	{
		friend class Drawable;
	public:
		
		virtual VkVertexInputBindingDescription getBindingDescription() = 0;
		virtual std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() = 0;
		virtual void Compile() = 0;
		virtual void Update() = 0;
		virtual size_t getCount() = 0;
		virtual size_t getSize() = 0;
		virtual void* getData() = 0;
	protected:
		struct Transform_S {
			glm::vec3 translate = glm::vec3(0, 0, 0);
			glm::vec3 rotate = glm::vec3(0, 0, 0);
			glm::vec3 scale = glm::vec3(1, 1, 1);
		} transform_s;

		glm::mat4 getModelMatrix();

		std::shared_ptr<Graphics::Vulkan> vulkan_ptr;
		std::shared_ptr<Graphics::DescriptorSetCore> desc_ptr;


		std::map<std::string, shared_ptr<Graphics::Buffer>> buffer_ptrs;
		std::map<std::string, shared_ptr<Dcb::Buffer>> bufs;
	};

	class Mesh
	{
		friend class Model;
	public:
		Mesh(shared_ptr<Graphics::Vulkan> vulkan_ptr,
			shared_ptr<Graphics::RenderPass> renderpass_ptr,
			shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr,
			shared_ptr<Graphics::CommandQueue> cmdQueue_ptr,
			shared_ptr<Graphics::DescriptorSetCore> desc_ptr);
		void Bind(BindType type, std::shared_ptr<Bind::Bindable> elem);
		void Compile();
	private:
		std::unique_ptr<Drawable> draw_ptr;
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

	class Model : public ModelBase
	{
	public:

		Model(std::shared_ptr<Graphics::Vulkan> vulkan_ptr, std::string file_path);

		void ParseMesh(const aiMesh& mesh);

		std::unique_ptr<Node> ParseNode(const aiNode& node);

		// Í¨¹ý ModelBase ¼Ì³Ð
		virtual VkVertexInputBindingDescription getBindingDescription() override;
		virtual std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() override;
		virtual void Compile() override;
		virtual void Update() override;
		virtual size_t getCount() override;
		virtual size_t getSize() override;
		virtual void* getData() override;


		void Register(shared_ptr<Control::Scene> scene_ptr)
		{
			this->scene_ptr = scene_ptr;
		}

		std::unique_ptr<Node> pRoot;
		vector < Mesh > meshes;

		shared_ptr<Graphics::Vulkan> vulkan_ptr;
		shared_ptr<Graphics::RenderPass> renderpass_ptr;
		shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr;
		shared_ptr<Graphics::CommandQueue> cmdQueue_ptr;
		
		shared_ptr<Control::Scene> scene_ptr;

	};
}



#endif // !MODEL_H
