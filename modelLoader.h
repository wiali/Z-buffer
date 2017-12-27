#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <iostream>
#include <string>
#include <vector>

#include "assimp/Importer.hpp"  // C++ importer interface
#include "assimp/postprocess.h" // Post processing fla
#include "assimp/scene.h"       // Output data structure

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

typedef struct {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
} Vertex;

class Mesh {
public:
  /*  Mesh Data  */
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
};

class ModelLoader {
public:
  ModelLoader(std::string path);

public:
  /* Model data */
  std::vector<Mesh> meshes;

private:
  void loadModel(std::string path);
  void processNode(aiNode *node, const aiScene *scene);
  Mesh processMesh(aiMesh *mesh, const aiScene *scene);

  const std::vector<Mesh> &GetMeshData();
};
#endif