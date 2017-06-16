#include <iostream>
#include <set>
#include "helper/profiling.h"

using namespace std;
using namespace leo;

int main() {
  set<int> s;
  const static int RUN = 1<< 22;
  for(int i=0;i<RUN;++i) s.insert(-i);
  start_profile_nonrecur("go through set");
  //using simple operation to prevent from being optimized out
  int ret = 0;
  do {
    auto it = s.cbegin();
    for(int i=0;i<RUN;++i) {
      ret += *it;
      ++it;
    }
  } while(false);
  stop_profile_nonrecur();
  start_profile_nonrecur("inc then dec");
  do {
    auto it = s.cbegin();
    for(int i=0;i<RUN/2;++i){
      ret += *it;
      ++it;
      ret += *it;
      --it;
    }
  } while(false);
  stop_profile_nonrecur();
  cout<<"just a dummy output: " << ret;
  return 0;
}
