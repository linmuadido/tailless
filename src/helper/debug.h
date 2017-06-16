#pragma once
#include <iostream>

#ifndef NDEBUG
inline void check_point(const char* filename, const int32_t line) {
  std::cout << filename << " : " << line << '.'<<std::endl;
}
#define CHECK_POINT() check_point(__FILE__,__LINE__) 
#else
#define CHECK_POINT()
#endif

