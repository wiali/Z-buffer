#ifndef Z_BUFFER_H
#define Z_BUFFER_H

#include <cstdlib>
#include <iostream>
#include <list>

#include "common.h"

// Z-Buffer扫描器
class ScanLineZBuffer {
public:
  ScanLineZBuffer(int width, int height, int depth,
                  glm::vec3 color = glm::vec3(1.0f));

  ~ScanLineZBuffer();

  char *flushBuffer(const std::vector<Mesh> &, const glm::mat4 &trans);

private:
  void ConstructClassifiedTable(const std::vector<Mesh> &,
                                const glm::mat4 trans);
  void DestroyClassifiedTable();

  void ZBuffer();

  void DestroyPolygon(ClassifiedPolygon *polygon);

  void SetPixel(int x, int y, glm::vec3 color);

private:
  int width;
  int height;
  int depth;
  RGB888 *buffer;
  int size;
  std::vector<std::list<ClassifiedPolygon *>> classifiedPolygonTable;

  // 保存转换的矩阵，避免重复计算
  glm::mat4 lastTrans;

  // 标识分类表是否为空
  bool empty;

  // 物体本色
  glm::vec3 objectColor;
};

#endif