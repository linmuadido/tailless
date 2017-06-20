#include "bst.h"
#include "avl.h"
#include "rb_tree.h"
#include "splay_tree.h"
#include <set>
#include "helper/debug.h"
#include "helper/compiler.h"
#include "helper/profiling.h"
using namespace std;
using namespace leo;



//constant declaration
#ifndef TEST_SIZE
#define TEST_SIZE (1<<16)
#endif
const static uint32_t run = TEST_SIZE;

#ifdef TEST_STD
using tut = set<int>;
#elif defined (TEST_BST)
using tut = bst<int>;
#elif defined (TEST_TH_BST)
using tut = threaded_bst<int>;
#elif defined (TEST_AVL)
using tut = avl<int>;
#elif defined (TEST_BIDIR_AVL)
using tut = bidir_avl<int>;
#elif defined (TEST_RB)
using tut = rb_tree<int>;
#elif defined (TEST_BIDIR_RB)
using tut = bidir_rb_tree<int>;
#elif defined (TEST_SPLAY)
using tut = splay_tree<int>;
#elif defined (TEST_BASE)
/*nothing*/
#else
#pragma message ("type under test not specified...fall back to std")
#define TEST_STD
using tut = set<int>;
#endif

#ifndef TEST_BASE
void __attribute__((noinline)) once(tut& t, int x) {
  t.insert(x);
}
#endif
using golden = set<int>;


#if !defined(TEST_STD) && !defined(TEST_BASE)
template<class golden, class type>
void verify(golden& g, type& t) {
  auto v = t.collect();
  if(v.size() !=g.size()) {
    cout<<"inconsisent size against golden"<<endl;
    exit(1);
  }
  int idx = 0;
  for(auto x : g) {
    if(v[idx++] == x) {
      continue;
    }
    cout<<"inconsistent order against golden"<<endl;
    exit(1);
  }
}
//TODO: separate "test" and "benchmark"
void test() {
  golden g;
  tut t;
  const static int RUN = 1<< 10;
  vector<int> arr;
  for(int i=0;i<RUN;++i) arr.push_back(rand());
  for(auto x : arr) {
    g.insert(x);
    t.insert(x);
    verify(g,t);
  }
  for(int i=0;i<RUN;++i) swap(arr[i],arr[rand()%(0u+RUN)]);
  for(auto x : arr) {
    if(g.erase(x) != t.erase(x)) {
      cout<<"erasing return value broken..."<<endl;      
    }
    verify(g,t);
  }
  arr.clear();
  arr = {1,3,2,4};
  for(int i=0;i<RUN;++i) arr.push_back(rand());
  for(auto x : arr) {
    g.insert(x);
    t.insert(x);
    verify(g,t);
  }
  for(auto x : arr) {
    if(g.erase(x) != t.erase(x)) {
      cout<<"erasing return value broken..."<<endl;      
    }
    verify(g,t);
  }

}
#else
void test(){}
#endif


int main(int argc, char** argv) {

  //for(int i=0;i<(1<<10);++i) test();

  vector<int> v(run);
  for(auto& x : v) x = rand();
#ifndef TEST_BASE

  tut t;

  start_profile_nonrecur("insertion");
  for(auto x : v) {
    //once(t,x);
    t.insert(x);
    //if(!t.verify())exit(1);
    //else cout<<"*"<<flush;
  }
  stop_profile_nonrecur();
#if defined( TEST_AVL ) || defined (TEST_BIDIR_AVL) || defined( TEST_RB ) || defined (TEST_BIDIR_RB)
//#pragma message ("compiling with one-time verification")
  //if(!t.verify())exit(1);
#endif
  for(auto i=0u;i<v.size();++i) {
    std::swap(v[i],v[rand()%TEST_SIZE]);
  }
  start_profile_nonrecur("deletion");
  for(auto x : v) {
    //once(t,x);
    t.erase(x);
    //if(!t.verify())exit(1);
    //else cout<<"*"<<flush;
  }
  stop_profile_nonrecur();
#endif
  return 0;
}
