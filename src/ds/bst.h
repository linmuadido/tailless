#pragma once
#include <cstddef>
#include <vector>
#include <iostream>
#include "./tree_node.h"
using namespace std;



namespace leo {
template<typename type>
class bst {
  typedef simple_node<type> node;
  public:
  bst() : root_() {}
  bool insert(const type& t) {
    //return insert(root_,t);
    if(root_ == NULL) {
      root_ = new node(t);
      return true;
    }
    node** to_insert = &root_;
    node*  n = root_;
    int idx;
    do {
      if(n->data_ < t) {
        idx = 1;
      } else if(t < n->data_) {
        idx = 0;
      } else return false;
      to_insert = &(n->children_[idx]);
      n = n->children_[idx];
    }while(n);
    *to_insert = new node(t);
    return true;
  }
  bool erase(const type& t) {
    return erase(root_,t);
  }
  vector<type> collect() const {
    vector<type> ret;
    collect(root_,ret);
    return ret;
  }
  private:
  node *root_;
  static bool insert(node*& n, const type& t) {
    if( n == NULL ) {
      n = new node(t);
      return true;
    }
    if(n->data_ < t) return insert(n->r_,t);
    if(t < n->data_) return insert(n->l_,t);
    return false;
    if( n->data_ == t) return false;
    return insert( n->data_ < t ? n->r_ : n->l_ , t);
  }
//#define PER_STEP_VERIFY
#ifndef PER_STEP_VERIFY
  static
#endif
  bool erase_node(node*& n) {
#ifdef PER_STEP_VERIFY
    std::vector<type> v1 = collect();
#endif
    node* to_delete = n;
    if((n->ptr_vals_[0] & n->ptr_vals_[1]) || n->ptr_vals_[0] * n->ptr_vals_[1]) {
      node** to_replace= &n->l_;
      node* n2 = n->l_;
      while(n2->r_) to_replace = &n2->r_, n2 = n2->r_;
      *to_replace = n2->l_;
      n2->l_ = n->l_;
      n2->r_ = n->r_;
      n = n2;
    } else {
      n = (node*)(n->ptr_vals_[0] ^ n->ptr_vals_[1]);
    }
#ifdef PER_STEP_VERIFY
    std::vector<type> v2 = collect();
    if(v1.size() != v2.size()+1) {
      cout<<"size error: "<<v1.size() << " vs " << v2.size() <<endl;
      exit(1);
    }
    for(int i=0, j=0;i<v1.size();++i) {
      if(v1[i] != v2[j]) {
        if(to_delete->data_ != v1[i]) {
          cout<<"data error"<<endl;
          exit(1);
        }
      } else ++j;
    }
#endif
    delete to_delete;
    return true;
  }
  bool erase(node*& n, const type& t) {
    if( n == NULL ) return false;
    node** next;
    if( n->data_ < t) {
      next = &n->r_;
    } else if(t < n->data_) {
      next = &n->l_;
    } else return erase_node(n);
    return erase(*next,t);
  }
  void collect(node* n, vector<type>& result) const {
    if(n == NULL)return;
    collect(n->l_,result);
    result.push_back(n->data_);
    collect(n->r_,result);
  }
};
}
