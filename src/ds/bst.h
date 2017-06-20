#pragma once
#include <cstddef>
#include <vector>
#include <iostream>
#include "./tree_node.h"
#include "helper/debug.h"
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
      node** to_replace, *n2;
#if 0 // always replace with node from left subtree
      to_replace= &n->l_;
      n2 = n->l_;
      while(n2->r_) to_replace = &n2->r_, n2 = n2->r_;
      *to_replace = n2->l_;
#else
      int idx = (n->l_ < n->r_);
      to_replace= &n->children_[!idx];
      n2 = n->children_[!idx];
      while(n2->children_[idx]) to_replace = &n2->children_[idx], n2 = n2->children_[idx];
      *to_replace = n2->children_[!idx];
#endif
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
template<typename type>
class threaded_bst {
  typedef simple_node<type> node;
  public:
  threaded_bst() {
    root() = sink()->l_ = sink()->r_ = threaded_sink();
  }
  bool insert(const type& t) {
    if(root() == threaded_sink()) {
      root() = new node(t,threaded_sink(),threaded_sink());
      sink()->l_ = sink()->r_ = make_thread(root());
      return true;
    }
    int idx;
    node* p;
    node* n = root();
    do {
      if(n->data_ < t) {
        idx = 1;
      } else if(t < n->data_) {
        idx = 0;
      } else return false;
      p = n;
      n = n->children_[idx];
    } while(is_threaded(n) == false);
    node* arr[2];
    arr[idx] = p->children_[idx];
    arr[!idx] = make_thread(p);
    node* new_node = new node(t,arr[0], arr[1]);
    p->children_[idx] = new_node;
    if(arr[idx] == threaded_sink()) {
      sink()->children_[!idx] = make_thread(new_node);
    }
#if 0
    vector<type> v1, v2;
    collect_recur(root(),v1);
    collect_threaded(sink()->r_,v2);
    if(v1.size() - v2.size()) {
      cout<<"inconsistent size" << endl;
      for(auto x : v1)cout<<x<<' ';cout<<endl;
      for(auto x : v2)cout<<x<<' ';cout<<endl;
      exit(1);
    }
    for(int i=0;i<v1.size();++i) {
      if(v1[i] != v2[i]) {
        cout << "out of order" << endl;
        exit(1);
      }
    }
#endif
    return true;
  }
  bool erase(const type& t) {
    node* p = source();
    node* n = root();
    int idx = 1;
    while( !is_threaded(n) ) {
      if(n->data_ < t) idx = 1;
      else if(t < n->data_) idx = 0;
      else return erase(p,n,idx);
      p = n;
      n = n->children_[idx];
    }
    return false;
  }
  bool erase(node* p, node* n, int idx) {
    node* to_delete = n;
    bool thl = is_threaded(n->l_);
    bool thr = is_threaded(n->r_);
    if(thl && thr) {
      if(p == source()) {
        root() = sink()->l_ = sink()->r_ = threaded_sink();
      } else {
        if(n->children_[idx] == threaded_sink()) {
          sink()->children_[!idx] = make_thread(p);
        }
        p->children_[idx] = n->children_[idx];
      }
    } else { //note: thl ^ thr holds
      int idx2 = thl;
      node* p2 = n, *n2 = n->children_[idx2];
      while( !is_threaded(n2->children_[!idx2]) ) {
        p2 = n2;
        n2 = n2->children_[!idx2];
      }
      if(p2 == n) {
        if(n->children_[!idx2] == threaded_sink()) {
          sink()->children_[idx2] = make_thread(n2);
        }
        n2->children_[!idx2] = n->children_[!idx2];
        p->children_[idx] = n2;
      } else {
        if(is_threaded(n2->children_[idx2])) p2->children_[!idx2] = make_thread(n2);
        else p2->children_[!idx2] = n2->children_[idx2];
        if(!is_threaded(n->children_[!idx2])) {
          node* tmp = n->children_[!idx2];
          while( !is_threaded(tmp->children_[idx2]) ) {
            tmp = tmp->children_[idx2];
          }
          tmp->children_[idx2] = make_thread(n2);
        } else if(n->children_[!idx2] == threaded_sink()) {
          sink()->children_[idx2] = make_thread(n2);
        }
        n2->l_ = n->l_;
        n2->r_ = n->r_;
        p->children_[idx] = n2;
      }
    }
    delete to_delete;
    return true;
  }
  vector<type> collect() const {
    vector<type> ret;
    collect_threaded(sink()->r_,ret);
    return ret;
  }
  private:
  union {
    char sink_placeholder_[sizeof(node)];
    node sink_[0];
    struct {
      char ch;
      char threaded_sink_[0];
    };
  };
  union {
    char source_placeholder_[sizeof(node)];
    node source_[0];
  };
  node* source() {return source_;}
  const node* source() const {return source_;}
  node*& root() {return source()->r_;}
  node* root() const {return source()->r_;} 
  node* sink() {return sink_;}
  const node* sink() const {return sink_;}
  node* threaded_sink() {return (node*)threaded_sink_;}
  const node* threaded_sink() const {return (node*)threaded_sink_;}
  void collect_recur(node* n, vector<type>& result) const {
    if(is_threaded(n)) return;
    collect_recur(n->l_,result);
    result.push_back(n->data_);
    collect_recur(n->r_,result);
  }
  void collect_threaded(node* n, vector<type>& result) const {
    while(n != threaded_sink()) {
      n = dethread(n);
DETHREADED:
      result.push_back(n->data_);
      n = n->r_;
      if(is_threaded(n))continue;
      while(!is_threaded(n->l_)) n = n->l_;
      goto DETHREADED;
    }
  }
  inline static bool is_threaded(node* n) {
    return uintptr_t(n) & 1;
  }
  inline static node* dethread(node* n) {
    return (node*)(uintptr_t(n) & -2);
  }
  inline static node* make_thread(node* n) {
    return (node*)(uintptr_t(n) | 1);
  }
};
}
