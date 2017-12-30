#ifndef DEBUG_H
#define DEBUG_H

#include <cstdlib>
#include <ctime>

#include "modelLoader.h"

std::vector<Mesh> meshes;

void debug_triangle() {

  int index[] = {0, 1, 2, 0, 1, 3, 1, 2, 3, 0, 2, 3};

  std::vector<Vertex> vertices;

  Vertex v;
  v.Position = glm::vec3(100, 100, 50);
  vertices.push_back(v);
  v.Position = glm::vec3(500, 300, 50);
  vertices.push_back(v);
  v.Position = glm::vec3(300, 400, 50);
  vertices.push_back(v);
  v.Position = glm::vec3(400, 200, 250);
  vertices.push_back(v);

  std::vector<unsigned int> indices(index, index + sizeof(index) / sizeof(int));

  Mesh mesh(vertices, indices);

  meshes.push_back(mesh);
  return;
}

void debug_cube() {

  int index[] = {0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 0, 7, 3, 0, 7, 4,
                 0, 5, 1, 0, 5, 4, 2, 5, 1, 2, 5, 6, 2, 7, 3, 2, 7, 6};

  std::vector<Vertex> vertices;

  Vertex v;
  v.Position = glm::vec3(-0.5, -0.5, -0.5);
  vertices.push_back(v);
  v.Position = glm::vec3(0.5, -0.5, -0.5);
  vertices.push_back(v);
  v.Position = glm::vec3(0.5, 0.5, -0.5);
  vertices.push_back(v);
  v.Position = glm::vec3(-0.5, 0.5, -0.5);
  vertices.push_back(v);
  v.Position = glm::vec3(-0.5, -0.5, 0.5);
  vertices.push_back(v);
  v.Position = glm::vec3(0.5, -0.5, 0.5);
  vertices.push_back(v);
  v.Position = glm::vec3(0.5, 0.5, 0.5);
  vertices.push_back(v);
  v.Position = glm::vec3(-0.5, 0.5, 0.5);
  vertices.push_back(v);

  std::vector<unsigned int> indices(index, index + sizeof(index) / sizeof(int));

  Mesh mesh(vertices, indices);

  meshes.push_back(mesh);
  return;
}

#endif