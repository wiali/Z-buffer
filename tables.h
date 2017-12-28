#ifndef TABLES_H
#define TABLES_H

#include <cstdint>
// 存放转换为整形的坐标
typedef struct {
  int x;
  int y;
  int z;
} RealVertex;

// RGBA color
typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RGB888;

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
  RGB888 color; // 多边形颜色

  Border *border[3];
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

  ClassifiedPolygon *source;

  DynamicBorder *next;
  // 存放第三条分类边
  Border *third;
};

#endif