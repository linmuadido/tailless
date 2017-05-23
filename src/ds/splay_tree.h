#pragma once
#include <cstddef>
#include <vector>
#include <iostream>
#include "tree_node.h"
using namespace std;


namespace leo {
template<typename type>
class splay_tree {
  typedef simple_node<type> node;
  public:
  splay_tree() : root_() {}
  bool insert(const type& t) {
    return insert(root_,t);
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
  static void rotate_to_right(node*& n) {
    node* tmp = n->l_;
    n->l_ = tmp->r_;
    tmp->r_ = n;
    n = tmp;
  }
  static void rotate_to_left(node*& n) {
    node* tmp = n->r_;
    n->r_ = tmp->l_;
    tmp->l_ = n;
    n = tmp;
  }
  bool insert(node*& n, const type& t) {
    if( n == NULL ) {
      n = new node(t);
      return true;
    }
    if( n->data_ == t) return false;
    if(n->data_ < t) {
      if( !insert( n->r_ , t ) )return false;
      rotate_to_left(n);
    }
    else {
      if( !insert( n->l_ , t ) )return false;
      rotate_to_right(n);
    }
    return true;
  }
  static bool bring_to_top(node*& n, const type& t) {
    if( n == NULL ) return false;
    if( n->data_ == t) return true;
    if(n->data_ < t) {
      if( !bring_to_top( n->r_ , t ) ) return false;
      rotate_to_left(n);
    }
    else {
      if( !bring_to_top( n->l_ , t ) ) return false;
      rotate_to_right(n);
    }
    return true;
  }
  static bool erase(node*& n, const type& t) {
    if( !bring_to_top(n,t) ) return false;
    node* toDelete = n;
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
    delete toDelete;
    return true;
  }
  void collect(node* n, vector<type>& result) const {
    if(n == NULL)return;
    collect(n->l_,result);
    result.push_back(n->data_);
    collect(n->r_,result);
  }
};
}
