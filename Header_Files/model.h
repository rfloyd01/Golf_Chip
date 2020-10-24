#pragma once

#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Header_Files/mesh.h>
#include <Header_Files/glm.h>
#include <Header_Files/shader.h>
#include <Header_Files/stb_image.h>

class Mesh;

class Model
{
public:
    Model(std::string path)
    {
        loadModel(path);
    }
    Model();
    void Draw(Shader& shader);
    void loadModel(std::string path);
private:
    // model data
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> textures_loaded;

    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    unsigned int TextureFromFile(const char* path, const std::string& directory);
};
