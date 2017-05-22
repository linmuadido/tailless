#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

string gen_compile(const vector<string>& defs) {
  string ret = "g++ -std=c++11 -O3 -o tmp.out ";
  for(const auto& d : defs) {
    ret += "-D";
    ret += d;
    ret += ' ';
  }
  ret += "simple_pool_test.cpp";
  return ret;
}
string gen_run(const char* filename) {
  string ret = "perf stat -d ./tmp.out >&";
  ret += filename;
  return ret;
}
void compile_then_run(const vector<string>& defs, const char* filename) {
  string compile = gen_compile(defs);
  cout<<"running " << compile << "...\n";
  system(compile.c_str());
  cout<<"Done"<<endl;
  string run = gen_run(filename);
  cout<<"running " << run << "...\n";
  system(run.c_str());
  cout<<"Done"<<endl;
}

int main() {
  compile_then_run({"USE_NEW"},"a");
  compile_then_run({"USE_LOCAL"},"b");
  compile_then_run({},"c");
}
