#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
using namespace std;

struct vertex_t {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
};

struct texture_t {
    unsigned int id;
    string type;
    string path;
};

struct mesh_t {
    vector<vertex_t> vertices;
    vector<unsigned int> indices;

};

struct model_t {
    vector<mesh_t>    meshes;
};

bool processMesh(aiMesh* mesh, model_t* model);
bool processNode(aiNode* node, const aiScene* scene, model_t* model);
void loadModel(std::string const& path, model_t* model);

void debug(model_t* model);
