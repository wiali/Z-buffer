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
    memset(p, 0xFF, size);

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

void ScanLineZBuffer::ConstructClassifiedTable(const std::vector<Mesh>& meshes, const glm::mat4 trans){

    // 获取片元组织方式
    int  stepLength = meshes.GetStepLength();

    // 分别记录两个表的尾指针
    vector<ClassifiedPolygon*> polygon(this->height, nullptr);    
    vector<ClassifiedBorder*> boder（this->height, nullptr);
    for(int i = 0; i != this->height; ++i){
        polygon[i] = classifiedPolygonTable[i];
        boder[i] = classifiedBorderTable[i];
    }

    // 遍历网格
    for(auto iter = meshes.begin(); iter != meshes.end(); ++iter){

        // 提领节点数据
        const std::vector<Vertex> &vertices = &(iter->vertices);
        // 提领下标索引
        const std::vector<unsigned int> & indices = &(iter->indices);
        int sizeOfIndices = indices.size();

        float ymax_surface; // 指向片元上的最大y值
        float ymin_surface;  
        glm::vec4 *ymax_line; // 指向边上的最大y值
        glm::vec4 *gmin_line;
        glm::vec4 target[stepLength]; // 存放转换后的真实坐标

        if(triangulated)
            for(int i = 0; i < sizeOfIndices; i+=stepLength){  // 按片元节点数步进
                
                for(int j = 0; j < stepLength; ++j){
                    /*
                     * 将片元的坐标转换为标准屏幕坐标
                     * 所有变换的矩阵trans
                     * 基尺 standardScale
                     * 顶点数据需要加上w分量，这样才能做矩阵乘运算
                     * w分量设为1.0,不对其它分量产生影响
                     */
                    target[j] = standardScale * trans * glm::vec4(vertices[indices[i+j]].Position, 1.0);
                }

                // 计算分类边表
                for(int j = 0; j < stepLength; ++j){

                    // 计算ymax_line、ymin_line
                    if(target[j].y > target[(j+1)mod(stepLength)].y){
                        ymax_line = target + j;
                        ymin_line = target + (j + 1)mod(stepLength);
                    }
                    else if(target[j].y < target[(j+1)mod(stepLength)].y){
                        ymin_line = target + j;
                        ymax_line = target + (j + 1)mod(stepLength);
                    }
                    else
                        continue;

                    ClassifiedBorder *p = static_cast<ClassifiedBorder*>(malloc(sizeof(ClassifiedBorder)));
                    p->id = i;
                    p->next = nullptr;
                    
                }
            }

    }
}