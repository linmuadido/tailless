#include "algo/sort.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include "helper/profiling.h"
using namespace leo;


const static size_t SZ = 1<<26;
#define INIT_TEST(type) \
do { std::vector<type> v(SZ),v2(SZ);for(auto& x:v) x=rand(); \
const char* TEST_NAME = TEST_CAT #type;\
start_profile_nonrecur(TEST_NAME);
#define UNINIT_TEST() \
stop_profile_nonrecur(); \
for(size_t i = 1;i<SZ;++i) if(v[i] < v[i-1]) return std::string( TEST_NAME )+" failed";\
}while(false);
#define TEST_CAT "radix sort"

std::string radix_sort_test() {


  INIT_TEST(uint8_t);
  auto f1 = [](uint8_t x) ->uint8_t {return x;};
  radix_sort_copy(v.begin(),v.end(),v2.begin(),f1);
  UNINIT_TEST();

  INIT_TEST(uint16_t);
  auto f1 = [](uint16_t x) ->uint8_t {return x;};
  auto f2 = [](uint16_t x) ->uint8_t {return x>>8;};
  radix_sort_copy(v.begin(),v.end(),v2.begin(),f1,f2);
  UNINIT_TEST();

  INIT_TEST(uint32_t);
  auto f1 = [](uint32_t x) ->uint8_t {return x;};
  auto f2 = [](uint32_t x) ->uint8_t {return x>>8;};
  auto f3 = [](uint32_t x) ->uint8_t {return x>>16;};
  auto f4 = [](uint32_t x) ->uint8_t {return x>>24;};
  radix_sort_copy(v.begin(),v.end(),v2.begin(),f1,f2,f3,f4);
  UNINIT_TEST();

  INIT_TEST(int8_t);
  auto f1 = [](int8_t x) ->uint8_t {return x+0x80;};
  radix_sort_copy(v.begin(),v.end(),v2.begin(),f1);
  UNINIT_TEST();

  INIT_TEST(int16_t);
  auto f1 = [](int16_t x) ->uint8_t {return x;};
  auto f2 = [](int16_t x) ->uint8_t {return (x+0x8000)>>8;};
  radix_sort_copy(v.begin(),v.end(),v2.begin(),f1,f2);
  UNINIT_TEST();

  INIT_TEST(int32_t);
  auto f1 = [](int32_t x) ->uint8_t {return x;};
  auto f2 = [](int32_t x) ->uint8_t {return x>>8;};
  auto f3 = [](int32_t x) ->uint8_t {return x>>16;};
  auto f4 = [](int32_t x) ->uint8_t {return (x+0x80000000)>>24;};
  radix_sort_copy(v.begin(),v.end(),v2.begin(),f1,f2,f3,f4);
  UNINIT_TEST();

  return TEST_CAT" passed";
}

std::string std_sort_test() {
#define STD_TEST(type) \
  INIT_TEST(type);\
  std::sort(v.begin(),v.end());\
  UNINIT_TEST();
  STD_TEST(uint8_t);
  STD_TEST(uint16_t);
  STD_TEST(uint32_t);
  STD_TEST(int8_t);
  STD_TEST(int16_t);
  STD_TEST(int32_t);
  return TEST_CAT" passed";
}

using std::cout;
using std::endl;
int main() {
  cout << radix_sort_test() << endl;
  cout << std_sort_test() <<endl;
  return 0;
}
