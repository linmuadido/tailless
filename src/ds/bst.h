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
    if(root_ == NULL) {
      root_ = new node(t);
      return true;
    }
    node* n = root_, *p;
    bool onLeft;
    // only use '<' operator... as STL
    do {
      if(n->data_ < t) {
        p = n;
        n = n->r_;
        onLeft = false;
      } else if( t < n->data_ ) {
        p = n;
        n = n->l_;
        onLeft = true;
      } else return false;
    } while(n != NULL);
    if(onLeft) p->l_ = new node(t);
    else p->r_ = new node(t);
    return true;
    //easy implementation... about 2.5% slower?
    //return insert(root_,t);
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
