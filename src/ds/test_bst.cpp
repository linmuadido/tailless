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
using tut = set<int>;
#endif

#ifndef TEST_BASE
void __attribute__((noinline)) once(tut& t, int x) {
  t.insert(x);
}
#endif


int main(int argc, char** argv) {


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
