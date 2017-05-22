#include "./simple_pool.h"
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

#if defined( USE_NEW) || defined( USE_MALLOC )
inline static tut* once() {
#ifdef USE_NEW
#pragma message ("testing ordinary new operator")
  return new tut;
#else
#pragma message ("testing ordinary malloc")
  return (tut*)malloc(sizeof(tut));
#endif
}
#else
#if defined( V3 )
#pragma message ("testing pool of v3")
using pool = simple_pool_v3<sizeof(tut),SZ,TEST_ALIGN>;
#elif defined( V2 )
#pragma message ("testing pool of v2")
using pool = simple_pool_v2<sizeof(tut),SZ,TEST_ALIGN>;
#else
#pragma message ("testing pool of v1")
using pool = simple_pool<sizeof(tut),SZ,TEST_ALIGN>;
#endif
static pool* pp = NULL;
inline static tut* once (pool& p) {
  return (tut*)p.alloc();
}
#endif

#ifndef NDEBUG
template<class type>
void report_clock(clock_t before, clock_t after, const type& name) {
  clock_t diff = after - before;
  double msec = diff * 1000.0 / CLOCKS_PER_SEC;
  cout<< "clock time ("<<name<<"): " << msec << " msecs" << '\n'; 
}
#else 
#define report_clock(...)
#endif

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

#if defined( USE_NEW) || defined( USE_MALLOC )
  for(int i=0;i<SZ;++i) {
    tut* volatile result = once();
    (void)(result);
  }
#else
  tut* result = NULL;
  int32_t count = 0;
  do {
    result = once(p);
  } while(result && ++count <= SZ);
#endif
  after = clock();
  report_clock(before,after,"alloc");
#if !defined (USE_NEW) && !defined( USE_MALLOC )
  if(count != SZ) {
    cout<<"cannot allocate objects as many times as expected...check code" << endl;
  }
#endif
  return 0 ;
}

