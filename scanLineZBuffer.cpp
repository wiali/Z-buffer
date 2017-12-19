#include <iostream>
#include <cstdlib>
#include "scanLineZBuffer.h"

ScanLineZBuffer::ScanLineZBuffer(int w, int h)
    : width(w)
    , height(h)
    , empty(false)
{
    // 固定分类表大小
    classifiedPolygonTable.resize(height, nullptr);
    classifiedBorderTable.resize(height, nullptr);

    // 创建buffer
    int size = width * height; 
    void *p = malloc(width * height);
    memset(p, 0x00, size);

    buffer = static_cast<int*>(malloc(size));
}

ScanLineZBuffer::~ScanLineZBuffer(){
    if(!empty)
        DestroyClassifiedTable();
    if(buffer)
        free(buffer);
}

void ScanLineZBuffer::DestroyClassifiedTable(){
    
    // 释放分类多边形表
    DestroyTable(classifiedPolygonTable);
    DestroyTable(classifiedBorderTable);
}

template<typename TABLE>
void ScanLineZBuffer::DestroyTable(std::vector<TABLE*> &table){

    for(auto iter = table.begin(); iter != table.end(); ++iter){
        if(iter){
            TABLE *head = iter->next;
            iter = nullptr;

            while(head){
                head = head->next;
                free(head);
            }
        }
    }
    return;
}

void ScanLineZBuffer::flushBuffer(const std::vector<Mesh>& meshes, const glm::mat4 trans){
    // 先确定变换矩阵是否变化
    // 无变化，则不需要进行转换，直接写屏
    if(empty && (lastTrans == trans))
        return;
    else if(!empty)
        DestroyClassifiedTable();

    lastTrans = trans;
    ConstructClassifiedTable(meshes , trans);
    empty = false;
}

inline static ClassifiedPolygon* caculateBorder(RealVertex *top, RealVertex *low, int tag){
    ClassifiedPolygon *p = static_cast<ClassifiedPolygon*>(malloc(sizeof(ClassfiedPolygon)));
    p->x = top->x;
    p->dx = float(low->x - top->x)/(top->y - low->y);
    p->dy = top->y - low->y;
    p->id = tag;
    return p;
} 

void ScanLineZBuffer::ConstructClassifiedTable(const std::vector<Mesh>& meshes, const glm::mat4 trans){

    // 获取片元组织方式, 已经固定三角形组织方式
    int  stepLength = 3;

    // 分别记录两个表的尾指针
    vector<ClassifiedPolygon*> polygon(this->height, nullptr);    
    vector<ClassifiedBorder*> border（this->height, nullptr);
    for(int i = 0; i != this->height; ++i){
        polygon[i] = classifiedPolygonTable[i];
        border[i] = classifiedBorderTable[i];
    }

    // 遍历网格
    for(auto iter = meshes.begin(); iter != meshes.end(); ++iter){

        // 提领节点数据
        const std::vector<Vertex> &vertices = &(iter->vertices);
        // 提领下标索引
        const std::vector<unsigned int> & indices = &(iter->indices);
        int sizeOfIndices = indices.size();

        // 保存转换后的节点坐标
        std::vector<RealVertex> realVertices(vertices.size());
        /*
        * 将片元的坐标转换为标准屏幕坐标
        * 所有变换的矩阵trans
        * 基尺 standardScale
        * 顶点数据需要加上w分量，这样才能做矩阵乘运算
        * w分量设为1.0,不对其它分量产生影响
        */
        glm::vec4 temp;
        for(int i = 0; i != vertices.size(); ++i){
            temp = standardScale * trans * glm::vec4(vertices[i].Position, 1.0);
            realVertices[i].x = temp.x;
            realVertices[i].y = temp.y;
            realVertices[i].z = temp.z;
        }

        RealVertex *ymax; // 指向三角形三个顶点顶点中y值最大点
        RealVertex *ymin;
        RealVertex *ymedium;

        for(int i = 0; i < sizeOfIndices; i+=stepLength){  // 按片元节点数步进

            // 确定次序
            ymin = realVertices + indices[i];
            ymedum = readVertices + indices[i + 1];
            if(ymin->y >= ymedium->y){
                ymin = ymedium;
                ymedium = realVertices + indices[i];
            }
            ymax = readVertices + indices[i + 2];
            if(ymin->y >= ymax->y){
                ymax = ymedium;
                ymedium = ymim;
                ymin = readVertices + indices[i + 2];
            }
            else if(ymedium->y >= ymax->y){
                ymax = ymedium;
                ymedium = readVertices + indices[i + 2];
            }

            // 计算分类边表
            ClassifiedPolygon *p = static_cast<ClassifiedPolygon*>(malloc(sizeof(ClassfiedPolygon)));
            p->id = i;
            p->dy = ymax->y - ymin->y;
            p->color = 0xFFFFFF;
            p->next = nullptr;    
            /*
             * 求平面系数, 点法式
             * A(x - x0) + B(y - y0) + C(z - z0) + D = 0
             *  展开即可
             */
            p->surface.a = (ymedium->y - ymin->y)*(ymax->z - ymin->z) - (ymax->y - ymin->y)*(ymediu->z - ymin->z);
            p->surface.b = (ymax->x - ymin->x)*(ymediu->z - ymin->z) - (ymediu->x - ymin->x)*(ymax->z - ymin->z);
            p->surface.c = (ymedium->x - ymin->x)*(ymax->y - ymin->y) - (ymax->x - ymin->x)*(ymedium->y - ymin->y);

            p->surface.d = -1 * (p->surface.a * ymin->x + p->surface.b * ymim->y + p->surface.c * ymin->z);
        
            // 入队
            polygon[ymax->y]->next = p;
            polygon[ymax->y] = polygon[ymax->y]->next;

            // 计算分类边表
            border[ymax->y]->next = caculateBorder(ymax, ymedium, i);
            border[ymax->y] = border[ymax->y]->next;

            border[ymax->y]->next = caculateBorder(ymax, ymin, i);
            border[ymax->y] = border[ymax->y]->next;

            // 加入的第三条线(最低位)应当不是一条水平线
            if(ymedium->y != ymin->y){
                border[ymedium->y]->next = caculateBorder(ymedium, ymin, i);
                border[ymedium->y] = border[ymedium->y]->next;
            }
        }

    }
}

void ScanLineZBuffer::ZBuffer(){
    // Z 缓冲器
    int *zbuffer = mallocwidth;

    // 活化多边形表
    DynamicPolygon* dynamicPolygonTable = nullptr;
    // 活化多边形表的尾部
    DynamicPolygon* tailPolygon = nullptr;

    // 活化边表
    DynamicBorder* dynamicBorderTable = nullptr;
    // 活化边表的尾部
    DynamicBorder* dynamicBorderTable = nullptr;

    // 从上向下扫描
    for(int sanline = hieght-1; sanline >= 0; --sanline){
        memeset(zbuffer, -1*width*height, width);
        // 从分类多边形表加入活化表
        ClassifiedPolygon* classifiedPolygon = classifiedPolygonTable[scanline]->next;
        while(classifiedPolygon){
            // 填充DynamicPolygon结构
            DynamicPolygon* p = static_cast<DynamicPolygon*>(malloc(sizeof(DynamicPolygon)));
            p->source =  &(classifiedPolygon->surface);
            p->dy = classifiedPolygon->dy;

            // 如果活化多边形表为空，则首位指针全部指向新建节点
            if(dynamicPolygonTable == nullptr){
                dynamicPolygonTable = p;
                tailPolygon = p;
            }
            else{
                // 否则，加入链表尾部
                tailPolygon->next = p;
                tailPolygon = tailPolygon->next;
            }

            // 分类多边形表指针后移
            classifiedPolygon = classifiedPolygon->next;
        }

        // 从分类边表加入活化边表
        ClassifiedBorder* classifedBorder = classifiedPolygonTable[scanline]->next;
        while(classifedBorder){
            // 填充DynamicBorder结构
            DynamicBorder* p = static_cast<DynamicBorder*>(malloc(sizeof(DynamicBorder)));
            p->xl = scanline;
            p->dxl = ;
            p->dyl = ;
            
        }
    }
    return;
}