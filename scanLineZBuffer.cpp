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
                std::free head;
            }
        }
    }
    return;
}