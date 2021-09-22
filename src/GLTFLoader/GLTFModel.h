#pragma once
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tinygltf/tiny_gltf.h>

#include "Drawable/DeferredMaterial.h"
#include "Drawable/Texture.h"

namespace GLTF
{
	enum FileLoadingFlags {
		None = 0x00000000,
		PreTransformVertices = 0x00000001,
		PreMultiplyVertexColors = 0x00000002,
		FlipY = 0x00000004,
		DontLoadImages = 0x00000008
	};

	enum class VertexComponent { Position, Normal, UV, Color, Tangent, Joint0, Weight0 };

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec4 color;
		glm::vec4 joint0;
		glm::vec4 weight0;
		glm::vec4 tangent;
		static VkVertexInputBindingDescription vertexInputBindingDescription;
		static std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
		static VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo;
		static VkVertexInputBindingDescription inputBindingDescription(uint32_t binding);
		static VkVertexInputAttributeDescription inputAttributeDescription(uint32_t binding, uint32_t location, VertexComponent component);
		static std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions(uint32_t binding, const std::vector<VertexComponent> components);
		/** @brief Returns the default pipeline vertex input state create info structure for the requested vertex components */
		static VkPipelineVertexInputStateCreateInfo* getPipelineVertexInputState(const std::vector<VertexComponent> components);
	};

	class Model
	{
		// The following structures roughly represent the glTF scene structure
// To keep things simple, they only contain those properties that are required for this sample
		struct Node;

		// A primitive contains the data for a single draw call
		struct Primitive {
			uint32_t firstIndex;
			uint32_t indexCount;
			int32_t materialIndex;
		};

		// Contains the node's (optional) geometry and can be made up of an arbitrary number of primitives
		struct Mesh {
			std::vector<Primitive> primitives;
		};

		// A node represents an object in the glTF scene graph
		struct Node {
			Node* parent;
			std::vector<Node> children;
			Mesh mesh;
			glm::mat4 matrix;
			std::string name;
			bool visible = true;
		};
	public:
		void loadFromFile(std::string filename, uint32_t fileLoadingFlags = FileLoadingFlags::None, float scale = 1.0f);
		void loadImages(tinygltf::Model& gltfModel);
		void loadMaterials(tinygltf::Model& gltfModel);
		void loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Model::Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<GLTF::Vertex>& vertexBuffer);
		std::string path;
		std::vector<Draw::DeferredMaterial> materials;
		std::vector<string> textureNames;
	};
}