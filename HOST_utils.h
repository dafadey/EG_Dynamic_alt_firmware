#include <string>

int charToHex(char a) {
  if (a>=0x30 && a<0x3A)
    return (int) a - 0x30;
  else if (a>=0x41 && a<=0x46)
    return (int) a - 0x41 + 10;
  else if (a>=0x61 && a<=0x66)
    return (int) a - 0x61 + 10;
  else
    return 16;
}

std::string tohex(unsigned char c) {
  std::string res = "0x";
  auto h=[](int x) -> unsigned char {
    return (unsigned char) (x < 10 ? x + 0x30 : x - 10 + 0x41);
  };
  res += h((int) c >> 4);
  res += h((int) c & 0xf);
  
  return res;
}
