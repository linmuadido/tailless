#include "bst.h"
#include "avl.h"
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
#else
#pragma message ("type under test not specified...fall back to std")
using tut = set<int>;
#endif



int main(int argc, char** argv) {


  vector<int> v(run);
  for(auto& x : v) x = rand();

  tut t;

  start_profile_nonrecur("insertion");
  for(auto x : v) t.insert(x);
  stop_profile_nonrecur();

  return 0;
}