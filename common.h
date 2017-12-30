#ifndef COMMON_H
#define COMMON_H

#include <cstdint>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// 顶点数据
typedef struct {
  // 位置信息
  glm::vec3 Position;
  // 点法向， 未被使用
  glm::vec3 Normal;
  // 纹理坐标， 未被使用
  glm::vec2 TexCoords;
} Vertex;

// 存放从模型中载入的网格数据
class Mesh {
public:
  /*  Mesh Data  */
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices) {
    this->vertices = vertices;
    this->indices = indices;
  }
};

// 整形格式颜色
typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RGB888;

// 存放转换为整形的坐标
typedef struct {
  int x;
  int y;
  int z;
} RealVertice;

// 多边形所在平面的方程系数
// ax+by+cz+d=0
struct Surface {
  int a;
  int b;
  int c;
  int d;
};

struct Border {
  float x;  // 边的上端点的x坐标
  float dx; // 相邻两条扫描线交点的x坐标差dx (-1/k)
  int dy;   // 边跨越的扫描线数目
};

struct ClassifiedPolygon {
  Surface surface;
  int dy;
  glm::vec3 color; // 片元本体的颜色
  glm::vec3 normal; // 片元法向，用于计算在光照下的颜色值， 未被使用

  Border *border[3]; // 分类多边形表直接提领三条边
};

struct DynamicBorder {
  float xl;  // 左交点的x坐标
  float dxl; // (左交点边上)两相邻扫描线交点的x坐标之差
  int dyl;
  float xr;
  float dxr;
  int dyr;

  float zl;
  float dzx;
  float dzy;

  ClassifiedPolygon *source; // 指向所在多边形

  DynamicBorder *next;

  Border *third; // 存放第三条分类边
};

#endif