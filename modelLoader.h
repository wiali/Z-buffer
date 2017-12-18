#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <iostream>
#include <vector>
#include <string>

#include "assimp/Importer.hpp"      // C++ importer interface
#include "assimp/scene.h"           // Output data structure
#include "assimp/postprocess.h"     // Post processing fla

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

typedef struct{
    glm::vec3 Postion;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
}Vertex;

typedef struct{
    unsigned int id;
    std::string type;
}Texture;

class Mesh{
public:
    /*  Mesh Data  */
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
};

class ModelLoader
{
public:
    ModelLoader(std::string path);
    void RenderModel();

private:
    /* Model data */
    std::vector<Mesh> meshes;
    std::string directory;

private:
    void loadModel(std::string path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    const std::vector<Mesh>&  GetMeshData();
};
#endif