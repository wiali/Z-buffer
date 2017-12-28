#include "scanLineZBuffer.h"
#include <cstdlib>
#include <iostream>

ScanLineZBuffer::ScanLineZBuffer(int w, int h, int d)
    : width(w), height(h), depth(d), empty(true) {
  // 固定分类多边形表大小
  classifiedPolygonTable.resize(height);

  // 创建buffer
  size = width * height * sizeof(RGB888);
  void *p = malloc(size);
  memset(p, 0x00, size);

  buffer = static_cast<RGB888 *>(p);
}

ScanLineZBuffer::~ScanLineZBuffer() {
  if (!empty)
    DestroyClassifiedTable();
  if (buffer)
    free(buffer);
}

void ScanLineZBuffer::DestroyClassifiedTable() {

  // 释放分类多边形表
  for (auto iter = classifiedPolygonTable.begin();
       iter != classifiedPolygonTable.end(); ++iter){
    for (auto pos = iter->begin(); pos != iter->end(); ++pos) {
      DestroyPolygon(*pos);
      *pos = nullptr;
    }
    if( !iter->empty() )
     iter->clear();
  }
}

void ScanLineZBuffer::DestroyPolygon(ClassifiedPolygon *polygon) {
  // 释放边
  for (int i = 0; i < 3; ++i)
    if (polygon->border[i]) {
      free(polygon->border[i]);
      polygon->border[i] = nullptr;
    }

  // 释放本体
  free(polygon);
}

char *ScanLineZBuffer::flushBuffer(const std::vector<Mesh> &meshes,
                                   const glm::mat4 trans) {
  // 先确定变换矩阵是否变化
  // 无变化，则不需要进行转换，直接写屏
  if (!empty && (lastTrans == trans))
    return reinterpret_cast<char *>(buffer);
  else if (!empty)
    DestroyClassifiedTable();

  // 清空buffer
  memset(reinterpret_cast<void *>(this->buffer), 0x00, size);

  lastTrans = trans;
  ConstructClassifiedTable(meshes, trans);
  ZBuffer();
  empty = false;
  return reinterpret_cast<char *>(buffer);
}

inline void ScanLineZBuffer::SetPixel(int x, int y, RGB888 color) {
  buffer[(height - y - 1) * width + x] = color;
}

inline static Border *caculateBorder(RealVertex *top, RealVertex *low) {
  Border *p = static_cast<Border *>(malloc(sizeof(Border)));
  p->x = top->x;
  p->dx = float(low->x - top->x) / (top->y - low->y);
  p->dy = top->y - low->y;
  return p;
}

inline static float caculateDepth(Surface *s, int x, int y) {
  return -1 * (s->a * x + s->b * y + s->d) / float(s->c);
}

inline static void clearZBuffer(int *buffer, int value, int w) {
  for (int i = 0; i < w; ++i)
    buffer[i] = value;
}

void ScanLineZBuffer::ConstructClassifiedTable(const std::vector<Mesh> &meshes,
                                               const glm::mat4 trans) {

  // 获取片元组织方式, 已经固定三角形组织方式
  int stepLength = 3;

  // 遍历网格
  for (auto iter = meshes.begin(); iter != meshes.end(); ++iter) {

    // 提领节点数据
    const std::vector<Vertex> &vertices = (iter->vertices);
    // 提领下标索引
    const std::vector<unsigned int> &indices = (iter->indices);
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
    for (unsigned int i = 0; i != vertices.size(); ++i) {
      temp = trans * glm::vec4(vertices[i].Position, 1.0);
      realVertices[i].x = (temp.x / temp.w + 0.5) * this->width;
      realVertices[i].y = (temp.y / temp.w + 0.5) * this->height;
      realVertices[i].z = (temp.z / temp.w + 0.5) * this->depth;
    }

    RealVertex *ymax; // 指向三角形三个顶点顶点中y值最大点
    RealVertex *ymin;
    RealVertex *ymedium;

    for (int i = 0; i < sizeOfIndices; i += stepLength) { // 按片元节点数步进

      // 确定次序
      ymin = realVertices + indices[i];
      ymedium = realVertices + indices[i + 1];
      if (ymin->y >= ymedium->y) {
        ymin = ymedium;
        ymedium = realVertices + indices[i];
      }
      ymax = realVertices + indices[i + 2];
      if (ymin->y >= ymax->y) {
        ymax = ymedium;
        ymedium = ymin;
        ymin = realVertices + indices[i + 2];
      } else if (ymedium->y >= ymax->y) {
        ymax = ymedium;
        ymedium = realVertices + indices[i + 2];
      }

      // 简单裁剪
      if (ymax->y >= this->height || ymin->y < 0)
        continue;

      // 计算分类多边形表
      ClassifiedPolygon *p =
          static_cast<ClassifiedPolygon *>(malloc(sizeof(ClassifiedPolygon)));
      p->dy = ymax->y - ymin->y;

      p->color.r = 0xFF;
      p->color.g = 0xFF;
      p->color.b = 0xFF;
      /*
       * 求平面系数, 点法式
       * A(x - x0) + B(y - y0) + C(z - z0) + D = 0
       *  展开即可
       */
      p->surface.a = (ymedium->y - ymin->y) * (ymax->z - ymin->z) -
                     (ymax->y - ymin->y) * (ymedium->z - ymin->z);
      p->surface.b = (ymax->x - ymin->x) * (ymedium->z - ymin->z) -
                     (ymedium->x - ymin->x) * (ymax->z - ymin->z);
      p->surface.c = (ymedium->x - ymin->x) * (ymax->y - ymin->y) -
                     (ymax->x - ymin->x) * (ymedium->y - ymin->y);

      // 该面在xoy平面上的投影是一条线，直接丢弃
      if (p->surface.c == 0) {
        free(p);
        continue;
      }

      p->surface.d = -1 * (p->surface.a * ymin->x + p->surface.b * ymin->y +
                           p->surface.c * ymin->z);

      // 指向polygon->border的首个空成员
      int pos_border = 0;

      // 填充边成员
      // 加入的第一条线(最高位)应当不是一条水平线
      if (ymax->y != ymedium->y) {
        Border *temp = caculateBorder(ymax, ymedium);
        p->border[pos_border++] = temp;
      }

      // ymax 与 ymin 构成的边总是合法的
      Border *temp = caculateBorder(ymax, ymin);
      p->border[pos_border++] = temp;

      // 加入的第三条(最低位)应当不是一条水平线
      if (ymedium->y != ymin->y) {
        Border *temp = caculateBorder(ymedium, ymin);
        p->border[pos_border++] = temp;
      }

      // 可能第三条边被丢弃，初始化指针
      if (pos_border == 2)
        p->border[pos_border] = nullptr;

      // 多边形入队
      classifiedPolygonTable[ymax->y].push_back(p);
    }
  }
}

void ScanLineZBuffer::ZBuffer() {
  // Z 缓冲器
  int zbuffer[width];

  // 活化边对表
  DynamicBorder *dynamicBorderTable = nullptr;
  // 活化边表队的尾部
  DynamicBorder *tailBorder = nullptr;
  // 指向片元的左右边
  Border *left_DB, *right_DB;

  // 从上向下扫描
  for (int scanline = this->height - 1; scanline >= 0; --scanline) {

    // 重置Z buffer缓冲器
    clearZBuffer(zbuffer, -1 * this->height * this->width, this->width);
    // 从分类多边形表加入活化表
    std::list<ClassifiedPolygon *> li = classifiedPolygonTable[scanline];

    // 按照1个多边形、两条或三条边的方式遍历
    for (auto iter = li.begin(); iter != li.end(); ++iter) {

      // 指向当前多边形
      ClassifiedPolygon *classifiedPolygon = *iter;

      // 判断左右边
      // 先默认认定0号边在1号边的左边
      left_DB = classifiedPolygon->border[0];
      right_DB = classifiedPolygon->border[1];

      /*
       * 在两种情况下，1号边在0号边左边，需要交换left、right位置
       * @1 left_DB->x > right_DB->x
       * @2 (left_DB->x == right_DB->x) && (left_DB->dx > right_DB->dx)
       */
      if ((left_DB->x > right_DB->x) ||
          ((left_DB->x == right_DB->x) && (left_DB->dx > right_DB->dx))) {
        left_DB = classifiedPolygon->border[1];
        right_DB = classifiedPolygon->border[0];
      }

      // 填充DynamicBorder结构
      DynamicBorder *q =
                static_cast<DynamicBorder *>(malloc(sizeof(DynamicBorder)));
      q->xl = left_DB->x;
      q->dxl = left_DB->dx;
      q->dyl = left_DB->dy;
      q->xr = right_DB->x;
      q->dxr = right_DB->dx;
      q->dyr = right_DB->dy;
      q->zl = caculateDepth(&(classifiedPolygon->surface), q->xl, scanline);
      q->dzx = -1 * classifiedPolygon->surface.a /
               float(classifiedPolygon->surface.c);
      q->dzy =
          classifiedPolygon->surface.b / float(classifiedPolygon->surface.c);
      q->source = classifiedPolygon;

      // 指向潜在的第三条边
      q->third = classifiedPolygon->border[2];
      q->next = nullptr;

      // 如果活化边表为空，则首位指针指向新建节点
      if (dynamicBorderTable == nullptr) {
        dynamicBorderTable = q;
        tailBorder = q;
      } else {
        // 否则，加入链表尾部
        tailBorder->next = q;
        tailBorder = tailBorder->next;
      }
    }

    // 开始进行深度计算，遍历活化边表
    DynamicBorder *cur, *pre = nullptr;
    cur = dynamicBorderTable;
    while( cur ) {
      // 对该活化边对进行向右递增的深度计算
      float zx = cur->zl;
      for (int ix = (cur->xl + 0.5); ix <= cur->xr; ++ix, zx += cur->dzx)
        if (ix >= 0 && ix < width) 
          if(zx > zbuffer[ix]){
            zbuffer[ix] = zx;
            SetPixel(ix, scanline, cur->source->color);
        }
      
      // 更新dy
      cur->dyl -= 1;
      cur->dyr -= 1;
      if (cur->dyl == 0 && cur->dyr == 0) {
        // 无第三条边可用、删除本活化边
        if(cur == dynamicBorderTable){
          dynamicBorderTable = cur->next;
          if(cur == tailBorder ){
            tailBorder = nullptr;
            free(cur);
            break;
          }
          free(cur);
          cur = dynamicBorderTable;
          continue;
        }
        else if(cur == tailBorder){
          tailBorder = pre;
          tailBorder->next = nullptr;
          free(cur);
          break;
        }
        else{
          cur = cur->next;
          free(pre->next);
          pre->next = cur;
          continue;
        }
      } else if (cur->dyl == 0) {
        // 左边已遍历完，换入第三条边
        cur->xl = cur->third->x;
        cur->dxl = cur->third->dx;
        cur->dyl = cur->third->dy;
        cur->third = nullptr;

        // 调整右边
        cur->xr = cur->xr + cur->dxr;
      } else if (cur->dyr == 0) {
        // 右边已遍历完，换入第三条边
        cur->xr = cur->third->x;
        cur->dxr = cur->third->dx;
        cur->dyr = cur->third->dy;
        cur->third = nullptr;

        // 调整左边
        cur->xl = cur->xl + cur->dxl;
      } else {
        // 调整双边
        cur->xl = cur->xl + cur->dxl;
        cur->xr = cur->xr + cur->dxr;
      }

      // 调整深度
      cur->zl = cur->zl + cur->dxl * cur->dzx + cur->dzy;

      // 调整cur 、 pre
      pre = cur;
      cur = cur->next;
    }
  }
  return;
}