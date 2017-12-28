#ifndef Z_BUFFER_H
#define Z_BUFFER_H

#include "modelLoader.h"
#include "tables.h"
#include <vector>
#include <list>

class ScanLineZBuffer {
public:
  ScanLineZBuffer(int width, int height, int depth);

  ~ScanLineZBuffer();

  char *flushBuffer(const std::vector<Mesh> &, const glm::mat4 trans);

private:
  void ConstructClassifiedTable(const std::vector<Mesh> &,
                                const glm::mat4 trans);
  void DestroyClassifiedTable();

  void ZBuffer();

  void DestroyPolygon(ClassifiedPolygon *polygon);

  void SetPixel(int x, int y, RGB888 color);

private:
  int width;
  int height;
  int depth;
  RGB888 *buffer;
  int size;
  std::vector<std::list<ClassifiedPolygon*> > classifiedPolygonTable;

  // 保存转换的矩阵，避免重复计算
  glm::mat4 lastTrans;

  // 标识分类表是否为空
  bool empty;
};

#endif