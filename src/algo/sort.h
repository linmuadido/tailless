#pragma once
#include <utility>
#include <algorithm>

namespace leo {

template< typename iter>
inline void radix_sort_copy_impl(iter b, iter e,iter out) {
  return;
}

template< typename iter, typename func, typename... other_funcs>
inline void radix_sort_copy_impl(iter b, iter e, iter out, func&& f, other_funcs&&... fs) {
  size_t arr[257] = {};
  size_t* const cnts = arr+1;
  for(auto it = b; it != e; ++it) {
    cnts[ f(*it) ] += 1;
  }
  size_t* const accs = arr;
  for(size_t tmp = 0, i=0;i<256;++i) {
    accs[i] = tmp += arr[i];
  }

  for(auto it = b; it != e;++it) {
    *(out + accs[f(*it)]++) = *it;
  }
  radix_sort_copy_impl(out,out+std::distance(b,e),b,std::forward<other_funcs>(fs)...);
}
template< typename iter, typename...funcs>
inline void radix_sort_copy(iter b, iter e, iter out, funcs&&... fs) {
  constexpr size_t num_runs = sizeof...(fs);
  radix_sort_copy_impl(b,e,out,std::forward<funcs>(fs)...);
  if(num_runs & 1) {
    std::copy(out,out+std::distance(b,e),b);
  }
}
template< typename iter>
inline void radix_sort_swap_impl(iter b, iter e) {
  return;
}

template< typename iter, typename func, typename... other_funcs>
inline void radix_sort_swap_impl(iter b, iter e, func&& f, other_funcs&&... fs) {
  size_t arr[257] = {};
  size_t* const cnts = arr+1;
  for(auto it = b; it != e; ++it) {
    cnts[ f(*it) ] += 1;
  }
  size_t* const lower = arr;
  union {
    size_t arr2[257];
    struct {
      size_t skip;
      size_t upper[256];
    } s;
  };
  auto&& upper = s.upper;
  for(size_t tmp = 0, i=0;i<256;++i) {
    arr2[i] = lower[i] = tmp += arr[i];
  }
  for(auto it = b; it != e;) {
    size_t& idx = lower[f(*it)];
    size_t bound = upper[f(*it)];
    if(idx < bound) {
      ++it;
    } else {
      std::swap(*(b+idx++),*it);
    }
  }
  radix_sort_swap_impl( b,e,std::forward<other_funcs>(fs)... );
}
template< typename iter, typename...funcs>
inline void radix_sort_swap(iter b, iter e, funcs&&... fs) {
  radix_sort_swap_impl(b,e,std::forward<funcs>(fs)...);
}

template< typename iter>
inline void heap_sort(iter b, iter e) {
  using std::make_heap;
  using std::pop_heap;
  make_heap(b,e);
  while(b<e) pop_heap(b,e--);
}
}
