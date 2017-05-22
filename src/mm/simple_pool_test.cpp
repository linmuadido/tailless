#include "simple_pool.h"
#include <iostream>
using namespace std;

using tut = union {
  int32_t x; // for assignment
  char arr[8];
};
//using tut = int64_t;
constexpr static int32_t SZ = 1<<26;
#ifndef TEST_ALIGN
#define TEST_ALIGN 0
#endif
#if defined( V4 )
using pool = simple_pool_v4<sizeof(tut),SZ,TEST_ALIGN>;
#elif defined( V3 )
using pool = simple_pool_v3<sizeof(tut),SZ,TEST_ALIGN>;
#elif defined( V2 )
using pool = simple_pool_v2<sizeof(tut),SZ,TEST_ALIGN>;
#else
using pool = simple_pool<sizeof(tut),SZ,TEST_ALIGN>;
#endif
static tut* once_new() {
  return new tut;
}
static tut* once_malloc() {
  return (tut*)malloc(sizeof(tut));
}
static tut* once(pool& p) {
  return (tut*)p.alloc() ;
}



#if defined( USE_NEW) || defined( USE_MALLOC )
#else
static pool* pp = NULL;
#endif

template<class type>
void report_clock(clock_t before, clock_t after, const type& name) {
  clock_t diff = after - before;
  double msec = diff * 1000.0 / CLOCKS_PER_SEC;
  cout<< "clock time ("<<name<<"): " << msec << " msecs" << '\n'; 
}

int main() {
  clock_t before, after;

  before = clock();
#if defined( USE_NEW) || defined( USE_MALLOC )
#else
#ifdef USE_LOCAL
  pool p;
#else
  static pool p;
#endif
  after = clock();
  report_clock(before,after,"init");
  pp = &p;
#endif
  before = clock();
  uintptr_t ret = 0;
  for(int i=0;i<SZ;++i) {
    tut* volatile result = 
#if defined( USE_NEW)
    once_new()
#elif defined( USE_MALLOC )
    once_malloc()
#else
    once(p)
#endif
    ;
#ifdef DUMP_PTR
    cout<<result<<endl;
#endif
    //x += (uintptr_t)result;
    //y += result->x = 1;
    (void)(result);
  }
  after = clock();
  report_clock(before,after,"alloc");
  return 0 ;
}

