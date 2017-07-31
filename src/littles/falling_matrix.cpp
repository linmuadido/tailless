#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cctype>
#include <random>
#include <climits>

namespace leo {
struct rain_drop {
  int32_t b,e;
};
using byte_map = std::string;
inline std::pair<int32_t,int32_t> get_dimension() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return {w.ws_col,w.ws_row};
}
template<class gen>
inline std::string gen_rnd_string(int32_t len, gen&& g) {
  std::string lib;
  for(int i=0;i<256;++i) {
    if(isgraph(i))lib += char(i);
  }
  std::string ret;
  for(int i=0;i<len;++i) {
    ret += lib[g()%lib.size()];
  }
  return ret;
}
class rain {
  public:
  rain(int32_t w, int32_t h): w_(w),h_(h),matrix_(w*h,'*'),drops_(w,{ INT_MAX, INT_MAX }){}
  const byte_map& get_byte_map() const {return matrix_;}
  const std::vector<rain_drop>& get_drops() const {return drops_;}
  void next() {
    for(int i=0;i<w_;++i) {
      auto& d = drops_[i];
      if(d.b <h_) {
        ++d.b;
        ++d.e;
      } else {
        populate_at(i);
      }
      mutate_at(i);
    }
  }
  protected:
  void populate_at(int32_t col) {
    if(mt_() % w_ > 1) return;
    std::string tmp = gen_rnd_string(h_,mt_);
    for(int i=0;i<h_;++i) {
      matrix_[col*h_+i] = tmp[i];
    }
    int32_t len = h_ - (h_>>2);
    len += mt_() % (h_>>1);
    auto& d = drops_[col];
    d.e = 0;
    d.b = -len;
  }
  void mutate_at(int32_t col) {
    int32_t len = h_/18;
    std::string tmp = gen_rnd_string(len,mt_);
    for(auto x : tmp) {
      matrix_[col*h_+mt_()%h_] = x;
    }
  }
  int32_t w_,h_;
  byte_map matrix_;
  std::vector<rain_drop> drops_;
  std::mt19937 mt_;
};



template<class transformer, class painter>
class render {
  public:
  std::string operator()(const byte_map& matrix, const std::vector<rain_drop>& drops, int w, int h) const{
    const static transformer t;
    const static painter p;
    byte_map bm = t(matrix,drops,w,h);
    auto clrs = p(matrix,drops,w,h);
    int32_t curr = -1;
    std::string ret = "\033[1;1H";
    for(int i=0;i<w*h;++i) {
      if(curr != clrs[i]) {
        curr = clrs[i];
        if(sizeof(clrs[i]) == 1) {
          ret += "\033[38;5;";
          ret += std::to_string( curr );
        } else {//true color
          ret += "\033[38;2;";
          ret += std::to_string( (curr>>16)%0x100 );
          ret += ';';
          ret += std::to_string( (curr>>8)%0x100 );
          ret += ';';
          ret += std::to_string( (curr>>0)%0x100 );
        }
        ret += 'm';

      }
      ret += bm[i];
    }
    return ret;
  }
};

class prune_n_rotate {
  public:
  byte_map operator()(const byte_map& matrix, const std::vector<rain_drop>& drops, int w, int h) const {
    byte_map ret(matrix.size(),'*');
    for(int i=0;i<w;++i) for(int j=0;j<h;++j) {
      ret[j*w+i] = matrix[i*h+j];
    }
    for(int i=0;i<w;++i) {
      const auto& d = drops[i];
      for(int j = d.b < h ? d.b : h;--j>=0;) ret[j*w+i] = ' ';
      for(int j = d.e > 0 ? d.e : 0;j<h;++j) ret[j*w+i] = ' ';

    }
    return ret;
  }
};

class head_n_fade_256 {
  public:
  std::vector<uint8_t> operator()(const byte_map& matrix, const std::vector<rain_drop>& drops, int w, int h) const {
    std::vector<uint8_t> ret(w*h,46);
    for(int i=0;i<w;++i) {
      const auto& d = drops[i];
      for(int j=0;j<8;++j) {
        const static uint8_t arr[] = {
          0,
          22,
          28,
          28,
          34,
          70,
          40,
          76,
        };
        if(j+d.b<0) continue;
        if(j+d.b>=h)  break;
        ret[(j+d.b)*w+i] = arr[j];
      }
      for(int j=0;j<4;++j) {
        const static uint8_t arr[] = {
          120,
          157,
          194,
          231,
        };
        if(d.e-4+j<0) continue;
        if(d.e-4+j>=h) break;
        ret[(d.e-4+j)*w+i] = arr[j];
      }
    }
    return ret;
  }
};
class head_n_fade {
  public:
  std::vector<int32_t> operator()(const byte_map& matrix, const std::vector<rain_drop>& drops, int w, int h) const {
    std::vector<int32_t> ret(w*h,0x00ff00);
    for(int i=0;i<w;++i) {
      const auto& d = drops[i];
      for(int j=0;j<8;++j) {
        const static int32_t arr[] = {
          0x003300,
          0x193300,
          0x006600,
          0x336600,
          0x009900,
          0x4c9900,
          0x00cc00,
          0x66cc00,
        };
        if(j+d.b<0) continue;
        if(j+d.b>=h)  break;
        ret[(j+d.b)*w+i] = arr[j];
      }
      for(int j=0;j<4;++j) {
        const static int32_t arr[] = {
          0x87ff87,
          0xafffaf,
          0xd7ffd7,
          0xffffff,
        };
        if(d.e-4+j<0) continue;
        if(d.e-4+j>=h) break;
        ret[(d.e-4+j)*w+i] = arr[j];
      }
    }
    return ret;
  }
};



}

using namespace std;
using namespace leo;


int main() {
  auto p = get_dimension();
  int w = p.first;
  int h = p.second;
  cout<<"w: "<<w<<" ; h: "<<h<<endl;
  rain gen(w,h);
  std::vector<char> buffer(w*h*2);
  setvbuf(stdout,buffer.data(),_IOFBF,w*h*2);
#ifdef TUTU
  const static render<prune_n_rotate,head_n_fade_256> r;
#else
  const static render<prune_n_rotate,head_n_fade> r;
#endif
  while(1) {
    gen.next();
    const auto& m = gen.get_byte_map();
    const auto& d = gen.get_drops();
    std::string tmp = r(m,d,w,h);
    cout<<tmp;
    cout.flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //getchar();
  }
}





