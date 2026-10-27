#pragma once

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

#define STM_FLASH_PAGE_SIZE 256

struct stm32_bootloader {
  const std::array<std::array<std::string, 3>, 16> allcommands {{
        {"0x00", "get", "Gets the version and the allowed commands supported by the current version of the protocol."},
        {"0x01", "getversion", "Gets the protocol version."},
        {"0x02", "getid", "Gets the chip ID."},
        {"0x11", "readmemory", "Reads up to 256 bytes of memory starting from an address specified by the application."},
        {"0x21", "go", "Jumps to user application code located in the internal flash memory or in the SRAM."},
        {"0x31", "writememory", "(w) Writes up to 256 bytes to the RAM or flash memory starting from an address specified by the application."},
        {"0x43", "erase", "(e) Erases from one to all the flash memory pages."},
        {"0x44", "exterase", "Erases from one to all the flash memory pages using two-byte addressing mode (available only for USART bootloader v3.0 and higher)."},
        {"0x50", "special", "Generic command that allows to add new features depending on the product constraints, without adding a new command for every feature."},
        {"0x51", "extspecial", "Generic command that allows the user to send more data compared to the Special command."},
        {"0x63", "writeprotect", "Enables the write protection for some sectors."},
        {"0x73", "writeunprotect", "Disables the write protection for all flash memory sectors."},
        {"0x82", "readoutprotect", "Enables the read protection."},
        {"0x92", "readoutunprotect", "Disables the read protection."},
        {"0xa1", "getchecksum", "Computes a CRC value on a given memory area with a size multiple of 4 bytes."},
        {"0x7f", "start", "(s) Starts communication with stm32. You should use this command after connect."},
  }};

  UART port;
  std::array<char, 128> buff;
  const char ACK = 0x79;
  const char NACK = 0x1F;
  std::map<std::string, char> all_commands_map;

  stm32_bootloader()
  {
    for(auto& record : allcommands) {
      char cmd = charToHex(record[0][2]) * 16 +charToHex(record[0][3]);
      all_commands_map[record[1]] = cmd;
    }
    //all_commands_map["start"] = (char) 0x7f;
    for(auto& item : all_commands_map)
      std::cout << item.first << " : " << tohex(item.second) << '\n';
  }
  
  bool connect(const char* dev = nullptr) {
    if(dev == nullptr)
      port.open("/dev/ttyUSB0");
    else
      port.open(dev);
    
    port.set(38400 /*baudrate*/,
             -1 /*flow_control -1 -- some default, 0 -- no control, 1 -- hw control, 2 -- software control*/,
             8 /*data bits 5,6,7,8*/,
             1 /*stopbits 1 one, 2 two*/,
             'e' /*parity 'n' -- no parity, 'o' -- odd parity, 'e' -- even parity, 's' -- space*/);
    return port.fd;
  }

  void disconnect() {
    if(port.fd)
      port.close();
  }

  bool start() {
    port.send(&all_commands_map["start"], 1);
    char res;
    small_delay(1e-5);
    while(port.recv(&res, 1, 0) != 0);
    small_delay(1e-5);
    port.recv(&res, 1);
    if(res==ACK)
      return true;
    if(res==NACK)
      return false;
    std::cout << "got: " << (int) (unsigned char) res << '\n';
    return false;
  }
  
  bool get() {
    char cmd = all_commands_map["get"];
    char sb = (char) 0xff - cmd;
    port.send(&cmd, 1);
    port.send(&sb, 1);
    char res;
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 1 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    port.recv(&res, 1);
    int n=res;
    std::vector<char> data(n);
    for(int i=0;i<=n;i++)
      port.recv(&data[i], 1);
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 2 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    std::cout << "got " << n+1 << " bytes\n";
    std::cout << "version: " << tohex(data[0]) << '\n';
    std::cout << "available commands are:\n";
    for(int i=1;i<data.size();i++) {
      for(auto& record : allcommands) {
        if(charToHex(record[0][2])*16+charToHex(record[0][3]) == (int) data[i])
          std::cout << '\t' << record[0] << " : " << record[1] << " : " << record[2] << '\n';
      }
    }
    return true;
  }

  bool writeunprotect() {
    char cmd = all_commands_map["writeunprotect"];
    char sb = (char) 0xff - cmd;
    port.send(&cmd, 1);
    port.send(&sb, 1);
    char op1;
    char op2;
    char ver;
    char res;
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 1 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    port.recv(&ver, 1);
    port.recv(&op1, 1);
    port.recv(&op1, 1);
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 2 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    std::cout << "version: " << tohex(ver) << '\n';
    std::cout << "option byte1: " << tohex(op1) << '\n';
    std::cout << "option byte2: " << tohex(op2) << '\n';
    return true;
  }

  bool getversion() {
    char cmd = all_commands_map["getversion"];
    char sb = (char) 0xff - cmd;
    port.send(&cmd, 1);
    port.send(&sb, 1);
    char op1;
    char op2;
    char ver;
    char res;
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 1 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    port.recv(&ver, 1);
    port.recv(&op1, 1);
    port.recv(&op1, 1);
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 2 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    std::cout << "version: " << tohex(ver) << '\n';
    std::cout << "option byte1: " << tohex(op1) << '\n';
    std::cout << "option byte2: " << tohex(op2) << '\n';
    return true;
  }

  bool getid() {
    char cmd = all_commands_map["getid"];
    char sb = (char) 0xff - cmd;
    port.send(&cmd, 1);
    port.send(&sb, 1);
    char n;
    char msb;
    char lsb;
    char res;
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 1 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    port.recv(&n, 1);
    port.recv(&msb, 1);
    port.recv(&lsb, 1);
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 2 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    std::cout << "N=" << (int) n << '\n'; 
    std::cout << "ver: " << msb*256 + lsb << '\n';
    return true;
  }
  
  bool writememory(uint32_t addr, uint8_t* buff, uint32_t sz) {
    std::cout << "--->writing from " << (void*) buff << " to " << addr << ' ' << sz << " bytes\n";
    
    char cmd = all_commands_map["writememory"];
    char sb = (char) 0xff - cmd;
    port.send(&cmd, 1);
    port.send(&sb, 1);
    char res;
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 1 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    uint8_t c[4];
    for(int i=0;i<4;i++)
      c[i] = (addr >> (8*(3-i))) & 0xFF;
    for(int i=0; i<4;i++)
      port.send((char*) &c[i], 1);
    uint8_t checksum = c[0] ^ c[1] ^ c[2] ^ c[3];
    port.send((char*) &checksum, 1);
    std::cout << "addr MSB->|" << (int) c[0] << '|' << (int) c[1] << '|' << (int) c[2] << '|' << (int) c[3] << "|<-LSB, XOR=" << (int) checksum << '\n';
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 2 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    uint8_t n = sz - 1;
    port.send((char*) &n, 1);
    checksum = n;
    for(int i=0;i<sz;i++)
      checksum ^= buff[i];
    port.send((char*) buff, sz);
    port.send((char*) &checksum, 1);

    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 3 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    std::cout << "<---done\n";
    return true;
  }
  
  bool readmemory(uint32_t addr, uint8_t* buff, uint32_t sz) {
    std::cout << "--->reading from " << (void*) buff << " to " << addr << ' ' << sz << " bytes\n";
    
    char cmd = all_commands_map["readmemory"];
    char sb = (char) 0xff - cmd;
    port.send(&cmd, 1);
    port.send(&sb, 1);
    char res;
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 1 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    uint8_t c[4];
    for(int i=0;i<4;i++)
      c[i] = (addr >> (8*(3-i))) & 0xFF;
    for(int i=0; i<4;i++)
      port.send((char*) &c[i], 1);
    uint8_t checksum = c[0] ^ c[1] ^ c[2] ^ c[3];
    port.send((char*) &checksum, 1);
    std::cout << "addr MSB->|" << (int) c[0] << '|' << (int) c[1] << '|' << (int) c[2] << '|' << (int) c[3] << "|<-LSB, XOR=" << (int) checksum << '\n';
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 2 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    
    uint8_t n = sz - 1;
    port.send((char*) &n, 1);
    checksum = ~n;
    port.send((char*) &checksum, 1);

    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 3 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }

    for(uint32_t i=0; i<sz; i++)
      port.recv((char*) &buff[i], 1);

    std::cout << "<---done\n";
    return true;
  }
  
  bool global_erase() {
    char cmd = all_commands_map["erase"];
    char sb = (char) 0xff - cmd;
    port.send(&cmd, 1);
    port.send(&sb, 1);
    char res;
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 1 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    uint8_t erase[2]{0xff, 0x00}; // global erase
    port.send((char*) &erase[0], 1);
    port.send((char*) &erase[1], 1);
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 2 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    return true;
  }
  
  bool ext_erase(int nbytes=0) {
    char cmd = all_commands_map["exterase"];
    char sb = (char) 0xff - cmd;
    port.send(&cmd, 1);
    port.send(&sb, 1);
    char res;
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 1 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    if(nbytes==0) {
      uint8_t erase[2]{0xff, 0xff}; // global erase
      port.send((char*) &erase[0], 1);
      port.send((char*) &erase[1], 1);
      uint8_t cs = 0; // checksum
      port.send((char*) &cs, 1);
    } else {
      uint16_t n = nbytes/STM_FLASH_PAGE_SIZE;
      uint8_t num[2];
      num[0] = (n >> 8) & 0xff;
      num[1] = n & 0xff;
      port.send((char*) num, 2);
      
      std::vector<uint8_t> page_numbers(2*(n+1));

      uint8_t cs = num[0] ^ num[1]; // checksum

      for(uint16_t i=0;i<n+1;i++) {
        page_numbers[2*i] = (i >> 8) & 0xff;
        page_numbers[2*i+1] = i & 0xff;
        cs ^= page_numbers[2*i] ^ page_numbers[2*i+1];
      }

      port.send((char*) page_numbers.data(), page_numbers.size());
      port.send((char*) &cs, 1);
    }
  
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 2 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    return true;
  }
  
  bool go(uint32_t addr) {
    std::cout << "--->go to " << addr << '\n';
    
    char cmd = all_commands_map["go"];
    char sb = (char) 0xff - cmd;
    port.send(&cmd, 1);
    port.send(&sb, 1);
    char res;
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 1 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    uint8_t c[4];
    for(int i=0;i<4;i++)
      c[i] = (addr >> (8*(3-i))) & 0xFF;
    for(int i=0; i<4;i++)
      port.send((char*) &c[i], 1);
    uint8_t checksum = c[0] ^ c[1] ^ c[2] ^ c[3];
    port.send((char*) &checksum, 1);
    std::cout << "addr MSB->|" << (int) c[0] << '|' << (int) c[1] << '|' << (int) c[2] << '|' << (int) c[3] << "|<-LSB, XOR=" << (int) checksum << '\n';
    port.recv(&res, 1);
    if(res!=ACK) {
      std::cerr << "failed to get ACK 2 " << (res==NACK ? "got NACK" : "got ???") << '\n';
      return false;
    }
    return true;
  }
  
};


bool stm32_programm_go(const char* file, const char* device = nullptr) {
  stm32_bootloader stm32;
  if(!stm32.connect(device)) {
    std::cout << "failed to connect to COM port \"" << (device ? device : "/dev/ttyUSB0") << "\"\n";
    return false;
  }
  
  std::cout << "reset MCU now!!!\n";
  for(int i=0;i<8;i++) {
    delay(500);
    std::cout << ' ' << i << std::flush;
  }
  std::cout << "hope you did it\n";
  std::cout << "connecting to MCU...\n";
  
  if(!stm32.start()) {
    std::cout << "failed to connect to stm32 boot loader\n";
    return false;
  }
  
  if(!stm32.get()) {
    std::cout << "failed to get commmand list\n";
    return false;
  }

  if(!stm32.getid()) {
    std::cout << "failed to get chip id\n";
    return false;
  }

  if(!stm32.getversion()) {
    std::cout << "failed to get protocol version\n";
    return false;
  }
  
  std::ifstream binf(file, std::ios_base::binary);
  std::vector<uint8_t> buffer;
  if(binf)
    buffer=std::vector<uint8_t>(std::istreambuf_iterator<char>(binf), {});
  binf.close();

  if(!buffer.size()) {
    std::cerr << "something gone wrong while reading binary or no such file\n";
    return false;
  }
  
  if(!stm32.ext_erase(buffer.size())) {
    std::cout << "failed to erase chip\n";
    return false;
  }
  
  std::cout << "binary size is " << buffer.size() << '\n';
  const uint32_t chunk = STM_FLASH_PAGE_SIZE;
  uint8_t checkbuff[chunk];
  const uint32_t sz = buffer.size();
  uint32_t addr = 0x08000000;
  for(uint32_t i = 0; i < sz; i += chunk) {
    uint32_t wrsize = (uint32_t) (i + (uint32_t) chunk <= sz ? chunk : sz % chunk);
    if(!stm32.writememory(addr + i, &buffer[i], wrsize)) {
      std::cout << "error writing page " << i / chunk << '\n';
      return false;
    }
    else {
      if(!stm32.readmemory(addr+i, checkbuff, wrsize))
        std::cerr << "check read error for page " << i / chunk << '\n';
      else {
        bool match=true;
        for(uint32_t j=0;j<wrsize;j++)
          match &= (checkbuff[j] == buffer[i+j]);
        if(!match) {
          std::cerr << "read data mismatch the written one for page " << i / chunk << '\n';
          return false;
        }
      }
    }
  }
  
  if(!stm32.go(0x08000000)) {
    std::cout << "failed to issue go command\n";
    return false;
  }
  
  stm32.disconnect();
  return true;
  
}
