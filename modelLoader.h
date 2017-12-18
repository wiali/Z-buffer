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

enum SurfacePatern{
    TRIANGULATE = 1 // 三角形组织方式
};

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

    GetStepLength(){
        return stepLength;
    }
    
    void RenderModel();

private:
    /* Model data */
    std::vector<Mesh> meshes;
    std::string directory;
    int stepLength; // 片元下标步长

private:
    void loadModel(std::string path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    const std::vector<Mesh>&  GetMeshData();
};
#endif