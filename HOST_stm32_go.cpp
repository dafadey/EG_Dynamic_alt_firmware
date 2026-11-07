#include "HOST_stm32_bootloader.h"

int main(int argc, const char** argv) {
  bool ok{};
  
  if(argc=1)
    ok = stm32_go();
  else if(argc==2)
    ok = stm32_go(argv[1]);

  if(ok)
    return 0;
  else
    return -1;
}
