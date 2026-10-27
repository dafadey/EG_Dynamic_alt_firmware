#pragma once
#include <chrono>


struct timer {
  std::chrono::time_point<std::chrono::high_resolution_clock> t0;
  void start() {
    t0 = std::chrono::high_resolution_clock::now();
  }
  double elapsed() {
    return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t0).count();
  }
};

void delay(int ms) {
  auto t0 = std::chrono::high_resolution_clock::now();
  while(true) {
    auto t1 = std::chrono::high_resolution_clock::now();
    if(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() > ms)
      break;
  }
}

void small_delay(double s) {
  auto t0 = std::chrono::high_resolution_clock::now();
  while(true) {
    auto t1 = std::chrono::high_resolution_clock::now();
    if(std::chrono::duration<double>(t1 - t0).count() > s)
      break;
  }
}
