#include "HOST_stm32_bootloader.h"

int main(int argc, const char** argv) {
  bool ok{};
  
  if(argc=1)
    ok = stm32_programm_go("firmware.bin");
  else if(argc==2)
    ok = stm32_programm_go(argv[1]);
  else if(argc==3)
    ok = stm32_programm_go(argv[1], argv[2]);
  if(ok)
    return 0;
  else
    return -1;
}
