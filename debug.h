#include <cstdlib>
#include <ctime>

char texture_data[800*600*3];
char* color_test(int width, int height) {
  std::srand(std::time(nullptr));
  // 填充vertice中的颜色分量
  for (int i = 0; i < height; ++i){
    char c = rand()%256;
    for (int j = 0; j < width; ++j){
        int index = 3*(i*width+j);
        texture_data[index] = c;
        texture_data[index + 1] = c;
        texture_data[index + 2] = c;
    }
  }
  return (char*)texture_data;
}