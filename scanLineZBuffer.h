#ifndef Z_BUFFER_H
#define Z_BUFFER_H

#include <vector>
#include <modelLoader.h>
#include "tables.h"

class ScanLineZBuffer
{
public:
    ScanLineZBuffer(int width, int height);

    ~ScanLineZBuffer();

    void flushBuffer(const std::vector<Mesh>&, const glm::mat4 trans);

private:
    void ConstructClassifiedTable(const std::vector<Mesh>&, const glm::mat4 trans);
    void DestroyClassifiedTable();

    void ZBuffer();

    template<typename TABLE>
    void DestroyTable(std::vector<TABLE*> &table);

    void SetPixel(int x, int y);

private:
    int width;
    int height;
    int *buffer;
    std::vector<ClassifiedPolygon*> classifiedPolygonTable;
    std::vector<ClassifiedBorder*> classifiedBorderTable;
    
    // 保存转换的矩阵，避免重复计算
    glm::mat4 lastTrans;

    // 标识分类表是否为空
    bool empty; 

    // 计算坐标的基尺
    int standardScale;
};

#endif