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
    node* n = root_, *p;
    bool on_left;
    // only use '<' operator... as STL
    do {
      //this branch achieve a look-ahead : 
      //to tell if n is possibly null or not in the next iteration
      //looks like meaningless, but it is good for cpu pipelining and branch prediction
      if(uintptr_t(n->l_) & uintptr_t(n->r_)) {
        if(n->data_ < t) {
          on_left = false;
        } else if( t < n->data_ ) {
          on_left = true;
        } else return false;
        p = n;
        n = n->children_[on_left];
        continue;
      }
      if(n->data_ < t) {
        on_left = false;
      } else if( t < n->data_ ) {
        on_left = true;
      } else return false;
      p = n;
      //(story continued) 
      //the value of n is conducted too late so the branch could induce stalled cycles
      n = n->children_[on_left];
    } while(n != NULL);
    p->children_[on_left] = new node(t);
    /*
    if(on_left) p->l_ = new node(t);
    else p->r_ = new node(t);
    */
    return true;
    //easy implementation... about 2.5% slower?
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
    if( n->data_ == t) return false;
    return insert( n->data_ < t ? n->r_ : n->l_ , t);
  }
  bool erase(node*& n, const type& t) {
    if( n == NULL ) return false;
    if( n->data_ == t) {
      node* to_delete = n;
      if(n->l_ == NULL) n = n->r_;
      else if(n->r_ == NULL) n = n->l_;
      else {
        node *prev = NULL, *curr = n->r_;
        while(curr->l_) prev = curr, curr = curr->l_;
        if(prev == NULL) curr->l_ = n->l_, n = curr;
        else {
          prev->l_ = curr->r_;
          curr->l_ = n->l_;
          curr->r_ = n->r_;
          n = curr;
        }
      }
      delete to_delete;
      return true;
    }
    return erase( n->data_ < t ? n->r_ : n->l_ , t);
  }
  void collect(node* n, vector<type>& result) const {
    if(n == NULL)return;
    collect(n->l_,result);
    result.push_back(n->data_);
    collect(n->r_,result);
  }
};
}
