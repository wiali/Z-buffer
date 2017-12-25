#ifndef TABLES_H
#define TABLES_H

// 多边形所在平面的方程系数
// ax+by+cz+d=0
struct Surface{
    int a;
    int b;
    int c;
    int d;
};

struct ClassifiedPolygon{
    Surface surface;
    int dy; 
    int color; // 多边形颜色
    ClassifiedPolygon *next;
} ;

struct DynamicPolygon{
    ClassifiedPolygon *source;
    int dy; // 多边形跨越的剩余扫描线数目
    DynamicPolygon *next;
};

struct ClassifiedBorder{
    float x;  // 边的上端点的x坐标
    float dx; // 相邻两条扫描线交点的x坐标差dx (-1/k)
    int dy; // 边跨越的扫描线数目
    ClassifiedPolygon *source; // 边所属多边形编号
    ClassifiedBorder *next;
} ;

struct DynamicBorder{
    float xl; // 左交点的x坐标
    float dxl; // (左交点边上)两相邻扫描线交点的x坐标之差
    int dyl;
    float xr;
    float dxr;
    int dyr;

    float zl;
    float dzx;
    float dzy;

    ClassifiedPolygon* source;

    DynamicBorder *next;
    // 存放第三条分类边
    ClassifiedBorder *third;
};

#endif