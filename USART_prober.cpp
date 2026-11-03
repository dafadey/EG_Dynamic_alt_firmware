#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <fstream>
#include <unistd.h>

#include "HOST_uart.h"
#include "HOST_utils.h"
#include "HOST_delay.h"

UART port;

int main() {
    port.open("/dev/ttyUSB0");
    
    if(!port.fd) {
      std::cerr << "failed to open port\n";
      return -1;
    }
    port.set(115200 /*baudrate*/,
             -1 /*flow_control -1 -- some default, 0 -- no control, 1 -- hw control, 2 -- software control*/,
             8 /*data bits 5,6,7,8*/,
             1 /*stopbits 1 one, 2 two*/,
             'e' /*parity 'n' -- no parity, 'o' -- odd parity, 'e' -- even parity, 's' -- space*/);

    char in;
    while(true) {
      int c = port.recv(&in, 1, 7);
      if(c==1)
        std::cout << tohex(in) << ' ' << std::flush;
      //else
      //  std::cout << "\n...\n";
    }
    port.close();
}
