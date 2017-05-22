#pragma once
#include <cstdlib>
#include <cstdint>
#include <utility>
#include <iostream>
#include "helper/debug.h"
#include "helper/compiler.h"
  using std::cout;
  using std::endl;

template<size_t size, size_t num, uint32_t alignment = 0>
class simple_pool {
  struct placeholder {
    int32_t indices[num];
    char    objects[num][alignment ? (size+alignment-1) / alignment * alignment : size];
  };
  public:
  simple_pool() {
    chunk_ = (placeholder*) malloc( total() );
    for(int i=0;i<num;++i) {
      chunk_->indices[i] = i;
    }
  }
  ~simple_pool() {
    free(chunk_);
  }
  void* alloc() {
    //design choice : return NULL or fallback?
    if(unlikely(!idx_)) return NULL;
    return chunk_->objects[ pop() ];
  }
  private:
  int32_t pop() {
    return chunk_->indices[ --idx_ ];
  }
  placeholder* chunk_;
  size_t idx_ = num;
  private:
  constexpr static size_t total() {
    return sizeof(placeholder);
  }
};
template<size_t size, size_t num, uint32_t alignment = 0>
class simple_pool_v2 {
  typedef char object[alignment ? (size+alignment-1) / alignment * alignment : size];
  union placeholder {
    int32_t next;
    object obj;
  };
  public:
  simple_pool_v2() {
    chunk_ = (placeholder*) malloc( total() );
    uintptr_t addr = (uintptr_t)chunk_+total();
    for(int i=num-1;i>=0;--i) {
      addr -= sizeof(placeholder);
      *(int32_t*)addr = i+1;
    }
  }
  ~simple_pool_v2() {
    free(chunk_);
  }
  void* alloc() {
    //design choice : return NULL or fallback?
    if(unlikely(idx_<0)) return NULL;
    return chunk_+pop();
  }
  private:
  placeholder* chunk_;
  int32_t idx_ = num-1;
  private:
  int32_t pop() {
    int32_t ret = idx_;
    idx_ = chunk_[ret].next; 
    return ret;
  }
  constexpr static size_t total() {
    return sizeof(placeholder) * num;
  }
};

template<size_t size, size_t num, uint32_t alignment = 0>
class simple_pool_v3 {
  using object = char[alignment ? (size+alignment-1)/ alignment * alignment : size];
  union placeholder {
    placeholder* next;
    object obj;
  };
  public:
  simple_pool_v3() {
    first_ = chunk_ = (placeholder*) malloc( total() );
    uintptr_t addr = uintptr_t(chunk_);
    for(int i=0;i<num-1;++i) {
      ((placeholder*)addr)->next = (placeholder*)(addr + sizeof(placeholder));
      addr += sizeof(placeholder);
      //chunk_[i].next = chunk_ +i+1;
    }
    dummy_.next = ((placeholder*)addr)->next = &dummy_;
  }
  ~simple_pool_v3() {
    free(chunk_);
  }
  void* alloc() {
    //design choice : return NULL or fallback?
    //if(unlikely(!first_)) return NULL;
    auto ret = first_;
    first_ = first_->next;
    return ret != first_ ? ret : NULL;
  }
  private:
  placeholder* chunk_;
  placeholder* first_;
  //TODO: space optimization
  placeholder  dummy_;
  private:
  constexpr static size_t total() {
    return sizeof(placeholder) * num;
  }
};


