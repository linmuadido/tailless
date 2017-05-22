#pragma once

#ifndef NDEBUG
inline void check_point(const char* filename, const int32_t line) {
  std::cout << filename << " : " << line << '.'<<std::endl;
}
#define CEHCK_POINT() check_point(__FILE__,__LINE) 
#else
#define CHECK_POINT()
#endif

