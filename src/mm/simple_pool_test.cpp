#include "./simple_pool.h"
#include <algorithm>
#include <iostream>
using namespace std;
using namespace leo;

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
void test () {
  pool p;
  tut** arr = new tut*[SZ];
  cout<<"[before freeing]"<<endl;
  for(int i=0;i<SZ;++i) {
    arr[i] = once(p);
    if(arr[i]) continue;
    cout<<"test failed: cannot get as many entries as expected" << endl; return;
  }
  for(int i=0;i<SZ;++i) {
    if(!p.alloc()) continue;
    cout<<"test failed: too many entries " << endl;return;
  }
  sort(arr,arr+SZ);
  for(int i=1;i<SZ;++i) {
    const size_t hop = TEST_ALIGN ? (sizeof(tut)+TEST_ALIGN-1)/TEST_ALIGN*TEST_ALIGN : sizeof(tut);
    if(uintptr_t(arr[i]) == uintptr_t(arr[i-1])+hop) continue;
    cout<<"test failed: memeory not contigous" << endl;return;
  }
  for(int i=0;i<SZ;++i) {
    int j = rand() % SZ;
    auto tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
  }
  for(int i=0;i<SZ;++i) {
    p.free(arr[i]);
  }
  cout<<"[after freeing]"<<endl;
  for(int i=0;i<SZ;++i) {
    arr[i] = once(p);
    if(arr[i]) continue;
    cout<<"test failed: cannot get as many entries as expected" << endl; return;
  }
  for(int i=0;i<SZ;++i) {
    if(!p.alloc()) continue;
    cout<<"test failed: too many entries " << endl;return;
  }
  sort(arr,arr+SZ);
  for(int i=1;i<SZ;++i) {
    const size_t hop = TEST_ALIGN ? (sizeof(tut)+TEST_ALIGN-1)/TEST_ALIGN*TEST_ALIGN : sizeof(tut);
    if(uintptr_t(arr[i]) == uintptr_t(arr[i-1])+hop) continue;
    cout<<"test failed: memeory not contigous" << endl;return;
  }
  cout<<"[test finished]"<<endl;
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
#ifdef VERIFY_POOL
  test();
#endif
#endif
  return 0 ;
}

