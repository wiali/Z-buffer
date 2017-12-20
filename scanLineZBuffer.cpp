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
        if(*iter){
            TABLE *head = iter;
            iter = nullptr;

            TABLE *next;
            while(head){
                next = head->next;
                free(head);
                head = next;
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

inline static ClassifiedBorder* caculateBorder(RealVertex *top, RealVertex *low, int tag){
    ClassifiedBorder *p = static_cast<ClassifiedBorder*>(malloc(sizeof(ClassifiedBorder)));
    p->x = top->x;
    p->dx = float(low->x - top->x)/(top->y - low->y);
    p->dy = top->y - low->y;
    p->id = tag;
    return p;
} 

inline static float caculateDepth(Surface *s, int x, int y){
    return -1*(s->a * x + s->b * y + s->d)/float(s->c);
}

void ScanLineZBuffer::ConstructClassifiedTable(const std::vector<Mesh>& meshes, const glm::mat4 trans){

    // 获取片元组织方式, 已经固定三角形组织方式
    int  stepLength = 3;

    // 分别记录两个表的尾指针
    std::vector<ClassifiedPolygon*> polygon(this->height, nullptr);    
    std::vector<ClassifiedBorder*> border(this->height, nullptr);

    // 遍历网格
    for(auto iter = meshes.begin(); iter != meshes.end(); ++iter){

        // 提领节点数据
        const std::vector<Vertex> &vertices = (iter->vertices);
        // 提领下标索引
        const std::vector<unsigned int> & indices = (iter->indices);
        int sizeOfIndices = indices.size();

        // 保存转换后的节点坐标
        RealVertex realVertices[vertices.size()];
        /*
        * 将片元的坐标转换为标准屏幕坐标
        * 所有变换的矩阵trans
        * 基尺 standardScale
        * 顶点数据需要加上w分量，这样才能做矩阵乘运算
        * w分量设为1.0,不对其它分量产生影响
        */
        glm::vec4 temp;
        for(int i = 0; i != vertices.size(); ++i){
            temp = trans * glm::vec4(vertices[i].Position, 1.0);
            realVertices[i].x = standardScale * temp.x;
            realVertices[i].y = standardScale * temp.y;
            realVertices[i].z = standardScale * temp.z;
        }

        RealVertex *ymax; // 指向三角形三个顶点顶点中y值最大点
        RealVertex *ymin;
        RealVertex *ymedium;

        for(int i = 0; i < sizeOfIndices; i+=stepLength){  // 按片元节点数步进

            // 确定次序
            ymin = realVertices + indices[i];
            ymedium = realVertices + indices[i + 1];
            if(ymin->y >= ymedium->y){
                ymin = ymedium;
                ymedium = realVertices + indices[i];
            }
            ymax = realVertices + indices[i + 2];
            if(ymin->y >= ymax->y){
                ymax = ymedium;
                ymedium = ymin;
                ymin = realVertices + indices[i + 2];
            }
            else if(ymedium->y >= ymax->y){
                ymax = ymedium;
                ymedium = realVertices + indices[i + 2];
            }

            // 计算分类边表
            ClassifiedPolygon *p = static_cast<ClassifiedPolygon*>(malloc(sizeof(ClassifiedPolygon)));
            p->id = i;
            p->dy = ymax->y - ymin->y;
            p->color = 0xFFFFFF;
            p->next = nullptr;    
            /*
             * 求平面系数, 点法式
             * A(x - x0) + B(y - y0) + C(z - z0) + D = 0
             *  展开即可
             */
            p->surface.a = (ymedium->y - ymin->y)*(ymax->z - ymin->z) - (ymax->y - ymin->y)*(ymedium->z - ymin->z);
            p->surface.b = (ymax->x - ymin->x)*(ymedium->z - ymin->z) - (ymedium->x - ymin->x)*(ymax->z - ymin->z);
            p->surface.c = (ymedium->x - ymin->x)*(ymax->y - ymin->y) - (ymax->x - ymin->x)*(ymedium->y - ymin->y);

            // 该面在xoy平面上的投影是一条线，直接丢弃
            if(p->surface.c == 0){
                free(p);
                continue;
            }

            p->surface.d = -1 * (p->surface.a * ymin->x + p->surface.b * ymin->y + p->surface.c * ymin->z);
        
            // 入队
            polygon[ymax->y] = p;
            if(classifiedPolygonTable[ymax->y] == nullptr)
                classifiedPolygonTable[ymax->y] = polygon[ymax->y];
            polygon[ymax->y] = polygon[ymax->y]->next;

            // 计算分类边表
            // 加入的第一条线(最高位)应当不是一条水平线
            if(ymax->y == ymedium->y){
                border[ymax->y] = caculateBorder(ymax, ymedium, i);
                if(classifiedBorderTable[ymax->y] == nullptr)
                    classifiedBorderTable[ymax->y] = border[ymax->y];
                border[ymax->y] = border[ymax->y]->next;
            }

            border[ymax->y] = caculateBorder(ymax, ymin, i);
            if(classifiedBorderTable[ymax->y] == nullptr)
                classifiedBorderTable[ymax->y] = border[ymax->y];
            border[ymax->y] = border[ymax->y]->next;

            // 加入的第三条(最低位)应当不是一条水平线
            if(ymedium->y == ymin->y){
                border[ymedium->y] = caculateBorder(ymedium, ymin, i);
                if(classifiedBorderTable[ymedium->y] == nullptr)
                    classifiedBorderTable[ymedium->y] == border[ymedium->y];
                border[ymedium->y] = border[ymedium->y]->next;
            }
        }

    }
}

void ScanLineZBuffer::ZBuffer(){
    // Z 缓冲器
    int *zbuffer = static_cast<int*>(malloc(width * sizeof(int)));

    // 活化多边形表
    DynamicPolygon* dynamicPolygonTable = nullptr;
    // 活化多边形表的尾部
    DynamicPolygon* tailPolygon = nullptr;

    // 活化边对表
    DynamicBorder* dynamicBorderTable = nullptr;
    // 活化边表队的尾部
    DynamicBorder* tailBorder = nullptr;
    // 指向片元的左右边
    ClassifiedBorder *left_DB, *right_DB;

    // 从上向下扫描
    for(int scanline = height-1; scanline >= 0; --scanline){
        memset(zbuffer, -1*width*height, width);
        // 从分类多边形表加入活化表
        ClassifiedPolygon* classifiedPolygon = classifiedPolygonTable[scanline];
        // 从分类边表加入活化边表
        ClassifiedBorder* classifiedBorder = classifiedBorderTable[scanline];

        // 按照1个多边形、两条或三条边的方式遍历
        while(classifiedPolygon){
            // 填充DynamicPolygon结构
            DynamicPolygon* p = static_cast<DynamicPolygon*>(malloc(sizeof(DynamicPolygon)));
            p->source = classifiedPolygon;
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

            // 判断左右边
            if(classifiedBorder->dx <= 0){
                left_DB = classifiedBorder;
                right_DB = classifiedBorder->next;
            }
            else{
                left_DB = classifiedBorder->next;
                right_DB = classifiedBorder;
            }
            // 调整classifeidBorder使其指向潜在的第三条未被交的线
            classifiedBorder = classifiedBorder->next->next;

            // 填充DynamicBorder结构
            DynamicBorder* q = static_cast<DynamicBorder*>(malloc(sizeof(DynamicBorder)));
            q->xl = left_DB->x;
            q->dxl = left_DB->dx;
            q->dyl = left_DB->dy;
            q->xr = right_DB->x;
            q->dxr = right_DB->dx;
            q->dyr = right_DB->dy;
            q->zl =  caculateDepth(&(classifiedPolygon->surface), q->xl, scanline);  
            q->dzx =  -1 * classifiedPolygon->surface.a / float(classifiedPolygon->surface.c);
            q->dzy = classifiedPolygon->surface.b / float(classifiedPolygon->surface.c);
            q->id =  classifiedPolygon->id;

            // 检测该面是否还有第三条边
            if(classifiedBorder->id == q->id){
                q->third = classifiedBorder;
                // 最后一次调整classifiedBorder指针，使其指向一个新片元
                classifiedBorder = classifiedBorder->next;
            }
            else
                q->third = nullptr;
            q->next = nullptr;

            // 如果活化边表为空，则首位指针指向新建节点
            if(dynamicBorderTable == nullptr){
                dynamicBorderTable = q;
                tailBorder = q;
            }
            else{
                // 否则，加入链表尾部
                tailBorder->next = q;
                tailBorder = tailBorder->next;
            }

            // 分类多边形表指针后移
            classifiedPolygon = classifiedPolygon->next;
        }

        // 开始进行深度计算，遍历活化边表
        for(DynamicBorder *head = dynamicBorderTable; head; head = head->next){
            // 对该活化边对进行向右递增的深度计算

        }

    }
    return;
}