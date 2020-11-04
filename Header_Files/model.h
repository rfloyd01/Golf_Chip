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

//global definitions
#define number_of_model_types 5;

//Classes, structs and enums defined in other header files
class Mesh;

//Structs and enums for this class
enum class ModelType
{
    //A list of the different types of objects that can be rendered
    //creating this enum class so that a single render map can be created for each
    //different mode. Storing everything in a single vector can make it tricky to know
    //which Model is kept at which location. By using a map, the golf club is now stored
    //in a location called club. So if I want to delete the club but keep everything else
    //on screen I can just delete all model data out from the [CLUB] location of the map

    CLUB = 0, //golf club to be rendered
    CHIP = 1, //sensor to be rendered during calibration mode
    BALL = 2, //golf ball to be rendered
    BACKGROUND = 3, //may want to render something that looks like a golf course in the future
    LINE_OBJECTS = 4 //may need to render things such as straight lines or circles for training purposes
};

//Class definition
class Model
{
public:
    //PUBLIC FUNCTIONS
    //Constructors
    Model(std::string path)
    {
        loadModel(path);
    }
    Model();

    //Setup Functions
    void loadModel(std::string path);
    void setScale(glm::vec3 s);
    void setLocation(glm::vec3 l);

    //Rendering Functions
    glm::vec3 getScale();
    glm::vec3 getLocation();
    void Draw(Shader& shader);
    
private:
    //PRIVATE FUNCTIONS
    //Data Processing Functions
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    unsigned int TextureFromFile(const char* path, const std::string& directory);

    //PRIVATE VARIABLES
    //Data Vectors
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> textures_loaded;

    //Rendering Variables
    glm::vec3 model_scale = { 1.0, 1.0, 1.0 }; //by default everything should be the same size as the model loaded
    glm::vec3 model_location = { 0.0, 0.0, 0.0 }; //by default all models should be loaded at the origin
};

//Helper Functions
ModelType modeltypeFromInt(int m);