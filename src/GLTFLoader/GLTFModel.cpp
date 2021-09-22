#include "GLTFLoader/GLTFModel.h"

namespace GLTF
{
	bool loadImageDataFunc(tinygltf::Image* image, const int imageIndex, std::string* error, std::string* warning, int req_width, int req_height, const unsigned char* bytes, int size, void* userData)
	{
		return false;
	}
	bool loadImageDataFuncEmpty(tinygltf::Image* image, const int imageIndex, std::string* error, std::string* warning, int req_width, int req_height, const unsigned char* bytes, int size, void* userData)
	{
		return false;
	}

	void Model::loadFromFile(std::string filename, uint32_t fileLoadingFlags, float scale)
	{
		tinygltf::Model gltfModel;
		tinygltf::TinyGLTF gltfContext;

		if (fileLoadingFlags & FileLoadingFlags::DontLoadImages) {
			gltfContext.SetImageLoader(loadImageDataFuncEmpty, nullptr);
		}
		else {
			gltfContext.SetImageLoader(loadImageDataFunc, nullptr);
		}

		size_t pos = filename.find_last_of('/');
		path = filename.substr(0, pos);

		std::string error, warning;
		bool fileLoaded = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, filename);

		std::vector<uint32_t> indexBuffer;
		std::vector<Vertex> vertexBuffer;

		if (fileLoaded) {
			if (!(fileLoadingFlags & FileLoadingFlags::DontLoadImages)) {
				loadImages(gltfModel);
			}

		}
	}

	void Model::loadImages(tinygltf::Model& gltfModel)
	{

		for (tinygltf::Image& image : gltfModel.images) {
			Draw::textureManager->CreateTexture(path + "/" + image.uri, path + "/" + image.uri);
			textureNames.emplace_back(path + "/" + image.uri);
		}
	}

	void Model::loadMaterials(tinygltf::Model& gltfModel)
	{
		materials.resize(gltfModel.materials.size());
		for (size_t i = 0; i < gltfModel.materials.size(); i++) {
			tinygltf::Material glTFMaterial = gltfModel.materials[i];
			// Draw::DeferredMaterial material;
			if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end()) {
				
			}

			if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end()) {
				materials[i].AddTexture(Draw::PBRTEXTURE_TYPE::ALBEDO, textureNames[glTFMaterial.values["baseColorTexture"].TextureIndex()]);
			}
			if (glTFMaterial.additionalValues.find("normalTexture") != glTFMaterial.additionalValues.end()) {
				materials[i].AddTexture(Draw::PBRTEXTURE_TYPE::NORMAL, textureNames[glTFMaterial.additionalValues["normalTexture"].TextureIndex()]);		
			}
		}
	}

	void Model::loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Model::Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<GLTF::Vertex>& vertexBuffer)
	{

	}

}