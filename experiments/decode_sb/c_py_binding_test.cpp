#include <iostream>

extern "C" {

  void init();
  uint8_t* poll_frames();
  void release();
}


int main(int argc, char * argv[]) {

  init();

  std::cout << static_cast<int>(*(poll_frames() + 0)) << std::endl;

  release();

  return 0;
}
