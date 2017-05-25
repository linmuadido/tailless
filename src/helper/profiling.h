#pragma once
#include <ctime>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>



namespace leo {


template<typename msg_type>
inline void report_clock_profile(const msg_type& msg, clock_t elapsed) {
  using std::cout;
  using std::endl;
  cout<< "clock profiler[ " << msg << " ] took ";
  cout<< (elapsed*1000.0/CLOCKS_PER_SEC);
  cout<< " msecs" << endl;
}

namespace clock_record_once {
  using std::string;
  thread_local string  name;
  thread_local clock_t clk;
}
inline void start_profile_nonrecur(const char* x) {
  using namespace std;
  clock_record_once::name = x;
  clock_record_once::clk  = clock();
}
inline void stop_profile_nonrecur() {
  using namespace std;
  clock_t elapsed = clock() - clock_record_once::clk;
  return report_clock_profile(clock_record_once::name, elapsed);
}

}
