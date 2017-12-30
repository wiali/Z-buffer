#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <iostream>
#include <string>
#include <vector>

#include "assimp/Importer.hpp"  // C++ importer interface
#include "assimp/postprocess.h" // Post processing fla
#include "assimp/scene.h"       // Output data structure

#include "common.h"

class ModelLoader {
public:
  ModelLoader(std::string path);

  const std::vector<Mesh> &GetMeshData();
  int GetFragNumber();

private:
  /* Model data */
  std::vector<Mesh> meshes;

private:
  void loadModel(std::string path);
  void processNode(aiNode *node, const aiScene *scene);
  Mesh processMesh(aiMesh *mesh, const aiScene *scene);
};
#endif