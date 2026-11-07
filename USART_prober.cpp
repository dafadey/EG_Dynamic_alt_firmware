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

    std::string s;
    const size_t n=16;
    char buff[n];
    char in;
    while(true) {
      std::cout << "your message:";
      std::cin >> s;
      if(s=="exit")
        break;

      //port.send((char*) s.c_str(), s.size());
      
      int i=0;
      
      for(; i < std::min(s.size(), n); i++)
        buff[i] = s.c_str()[i];
      for(; i < n; i++)
        buff[i] = ' ';
      port.send(buff, 16);
      std::cout << "sent!\n";
      i=0;
      
      while(true) {
        int c = port.recv(&in, 1, 1);
        if(c==1) {
          buff[i] = in;
          i++;
        } else
          break;
        if(i==16)
          break;
      }

      if(i==16) {
        for(i = 0; i < 16; i++)
          std::cout << buff[i];
//          std::cout << tohex(buff[i]) << ' ';
        std::cout << '\n';
      } else
        std::cout << "no responce... i=" << i << '\n';

      /*
      int c = port.recv(&in, 1, 7);
      if(c==1)
        std::cout << tohex(in) << ' ' << std::flush;
      */
    }
    port.close();
}
