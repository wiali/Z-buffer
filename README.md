本工程实现了计算机图形学中的扫描线Z buffer算法。

# 开发平台信息

 - 编程语言: C++
 - 操作系统: ubuntu 16.04
 - 编译器: g++  gcc

 # 代码说明
 
``` bash
.
├── camera.cpp  // 一个摄像头视角的类实现、支持视角转换
├── camera.h    
├── common.h    // 一些共有数据结构,包含了分类多边形、动态边等
├── debug.h
├── glad.c  // glad OpenGL加载库
├── include // 包含使用的glad、glm等库的头文件
├── Main.cpp    // 主文件
├── Makefile    // make编译
├── modelLoader.cpp // 加载模型
├── modelLoader.h
├── README.md
├── resource    // 为了节省压缩包空间，仅包含了一头狼的模型
├── scanLineZBuffer.cpp // 扫描线Z-Buffer实现
└── scanLineZBuffer.h
```

# Z-Buffer的相关数据结构说明

由于本工程在加载模型库时指定导入三角形片元，边的数量少，仅采用了`分类多边形表`和`活化边表`，增加了一些数据成员以减少多边形、边的搜素，具体如下:

 1. 分类多边形(`ClassifiedPolygon`)表中增加了三个指向三角形片元三条边(`Border`)的指针，舍弃分类边表。
 2. 活化边(`DynamicBorder`)表中增加指向分类多边形的指针成员，实现快速多边形查找；增加一个边指针用来保存第三条边，这样在活化边表需要换入第三条边时可以直接代入，不需要进行额外的搜索操作。

# 本地编译和执行

工程使用了assimp、glad、glm、glfw第三方库支持，其中glad、glm已经整合进工程，本地编译需要安装glfw、assimp库。

``` bash
# 安装必要库
apt install libglfw3-dev libassimp-dev -y

# 编译生成Z-Buffer可执行文件
make

# 执行, 例
./Z-Buffer resource/Wolf_obj.obj
```

# 执行结果

执行机为dell 灵越14r笔记本，使用mesa intel驱动。

### 狼 2756块片元

<div aligin=center><img src="https://github.com/zlseu-edu/Z-buffer/blob/master/wolf.jpg"></div>

### 轿车 182324块片元

<div aligin=center><img src="https://github.com/zlseu-edu/Z-buffer/blob/master/car.jpg"></div>