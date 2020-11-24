#include "pch.h"

#include <iostream>
#include <Header_Files/model.h>
#include <Header_Files/quaternion_functions.h>

//PUBLIC FUNCTIONS
//Constructors
Model::Model()
{
    //nothing happens with default initializer
}

//Setup Functions
void Model::loadModel(std::string path)
{
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);

    //after all nodes are processed calculate the hit box for the model
    setBoundingBox();
}
void Model::setScale(glm::vec3 s)
{
    model_scale = s;
}
void Model::setLocation(glm::vec3 l)
{
    model_location = l;
}
void Model::setRotation(glm::quat r)
{
    model_rotation = r;
}

//Rendering Functions
glm::vec3 Model::getScale()
{
    return model_scale;
}
glm::vec3 Model::getLocation()
{
    return model_location;
}
glm::quat Model::getRotation()
{
    return model_rotation;
}
void Model::Draw(Shader& shader)
{
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}

//Get Functions
std::vector<glm::vec3> Model::getBoundingBox()
{
    std::vector<glm::vec3> bb;
    bb.push_back(transformVertex(min_coordinates));  bb.push_back(transformVertex(max_coordinates));
    return bb;
}

//PRIVATE FUNCTIONS
//Data Processing Functions
void Model::processNode(aiNode* node, const aiScene* scene)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}
Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        // process vertex positions, normals and texture coordinates
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;

        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        vertices.push_back(vertex);
    }
    // process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // process material
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return Mesh(vertices, indices, textures);
}
std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture); // add to loaded textures
        }
    }
    return textures;
}
unsigned int Model::TextureFromFile(const char* path, const std::string& directory)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

//Collision Detection Functions
void Model::setBoundingBox()
{
    //go through all vertices in mesh to find min and max values for x, y, z
    //should only need to call this function when a model is first created
    for (int i = 0; i < meshes.size(); i++)
    {
        for (int j = 0; j < meshes[i].vertices.size(); j++)
        {
            //only care about the Position vector within the Vertex object
            float x_pos = meshes[i].vertices[j].Position[0];
            float y_pos = meshes[i].vertices[j].Position[1];
            float z_pos = meshes[i].vertices[j].Position[2];

            //check x
            if (x_pos > max_coordinates[0]) max_coordinates[0] = x_pos;
            else if (x_pos < min_coordinates[0]) min_coordinates[0] = x_pos;

            //check y
            if (y_pos > max_coordinates[1]) max_coordinates[1] = y_pos;
            else if (y_pos < min_coordinates[1]) min_coordinates[1] = y_pos;

            //check z
            if (z_pos > max_coordinates[2]) max_coordinates[2] = z_pos;
            else if (z_pos < min_coordinates[2]) min_coordinates[2] = z_pos;
        }
    }
}
glm::vec3 Model::transformVertex(glm::vec3 vertex)
{
    //order of operations for vertex transform is translate, rotate, then scale
    vertex *= model_location;
    QuatRotate(model_rotation, vertex);
    vertex *= model_scale;

    return vertex;
}

//Helper Functions
ModelType modeltypeFromInt(int m)
{
    if (m == 0) return ModelType::CLUB;
    else if (m == 1) return ModelType::CHIP;
    else if (m == 2) return ModelType::BALL;
    else if (m == 3) return ModelType::BACKGROUND;
    else if (m == 4) return ModelType::LINE_OBJECTS;
}