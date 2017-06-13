#pragma once
#include <cstddef>
#include <vector>
#include <iostream>
#include "./tree_node.h"
#include "helper/compiler.h"
#include "helper/debug.h"
using namespace std;


//#define RB_PROFILE_ROATATION

namespace leo {
template<typename type>
class rb_tree {
  typedef unsigned short tag_type;
  typedef tagged_node<type, tag_type> node;
  public:
  static int& lcnt() {static int x = 0;return x;}
  static int& rcnt() {static int x = 0;return x;}
  rb_tree() : root_(sink()) {
#ifdef RB_PROFILE_ROATATION
    lcnt() = rcnt() = 0;
#endif
  }
  bool insert(const type& t) {
    if(root_ == sink()) {
      root_ = new node(t,sink(),sink(),BLACK);
      return true;
    }

    node** traces[128];
    char   dirs[128];

    int sz = 0;
    node** n = &root_;
    do {
      if((*n)->data_ < t) {
        traces[sz] = n;
        n = &((*n)->r_);
        dirs[sz++] = 1;
      } else if(t < (*n)->data_) {
        traces[sz] = n;
        n = &((*n)->l_);
        dirs[sz++] = 0;
      } else {
        return false;
      }
    } while(*n != sink());
    *n = new node(t,sink(),sink(),RED);


    if(color(*traces[--sz]) == BLACK) return true;
    int to_fix = true;
    while(--sz >= 0) {
      if(sz) __builtin_prefetch(traces[sz-1],1,3);
      node*& n = *(traces[sz]);
      if(color(n) == RED) {
        to_fix = true;
        continue;
      }
      if(to_fix == false) return true;
      if(color(n->l_) == color(n->r_)) {
        promote_red(n);
        to_fix = false;
      } else if(dirs[sz] == 1) {
        n->tag_ = RED;
        if(color(n->r_->l_) == RED) promote_rl(n);
        else rotate_to_left(n);
        n->tag_ = BLACK;
        return true;
      } else {
        n->tag_ = RED;
        if(color(n->l_->r_) == RED) promote_lr(n);
        else rotate_to_right(n);
        n->tag_ = BLACK;
        return true;
      }
    }
    //using tertun code to control...
    //if(insert(root_,t) == FAIL) return false;
    root_->tag_ = BLACK;
    return true;
  }
  bool erase(const type& t) {
    //if( erase(root_,t) == FAIL) return false;
    //if(color(root_) == RED) root_->tag_ = BLACK;
    node**        traces[128];
    unsigned char dirs[128];
    node** n = &root_;
    int sz = 0;


    while(1) {
      if(*n == sink()) return false;
      if((*n)->data_ < t) {
        traces[sz] = n;
        n = &((*n)->r_);
        dirs[sz++] = 1;
      } else if(t < (*n)->data_) {
        traces[sz] = n;
        n = &((*n)->l_);
        dirs[sz++] = 0;
      } else {
        break;
      }
    }
    node* to_delete = *n;
    if((*n)->l_ == (*n)->r_) {
      *n = sink();
      if(color(to_delete) == RED) {
        goto REPAIR_DONE;
      }
    } else if((*n)->l_ == sink()) {
      *n = (*n)->r_;
      (*n)->tag_ = BLACK;
      goto REPAIR_DONE;
    } else if((*n)->r_ == sink()) {
      *n = (*n)->l_;
      (*n)->tag_ = BLACK;
      goto REPAIR_DONE;
    } else if((*n)->l_->r_ == sink()) {
      *n = (*n)->l_;
      (*n)->r_ = to_delete->r_;
      if(color(*n) == RED) {
        (*n)->tag_ = BLACK;
        goto REPAIR_DONE;
      }
      //left child is black
      (*n)->tag_ = color(to_delete);
      if((*n)->l_ != sink()) {
        (*n)->l_->tag_ = BLACK;
        goto REPAIR_DONE;
      }
      traces[sz] = n;
      dirs[sz++] = 0;
    } else {
      node** to_be_altered = n;
      traces[sz] = n;
      dirs[sz++] = 0;
      n = &((*n)->l_);
      int subtree_pos = sz;
      do {
        traces[sz] = n;
        dirs[sz++] = 1;
        n = &((*n)->r_);
      } while((*n)->r_ != sink());
      node* orphan = (*n)->l_;
      auto clr = color(*n);
      (*n)->l_ = (*to_be_altered)->l_;
      (*n)->r_ = (*to_be_altered)->r_;
      (*n)->tag_ = (*to_be_altered)->tag_;
      *to_be_altered = *n;
      traces[subtree_pos] = &((*n)->l_);
      *n = orphan;
      if(orphan != sink()) {
        orphan->tag_ = BLACK;
        goto REPAIR_DONE;
      }
      if(clr != BLACK) {
        goto REPAIR_DONE;
      }
    }
    while(--sz >= 0) {
      n = traces[sz];
      int idx = dirs[sz];
      node* sibling = (*n)->children_[!idx];
      if(color(*n) == RED) {
      } else if( color(sibling) == RED ) {
        if(!idx) rotate_to_left(*n);
        else rotate_to_right(*n);
        (*n)->tag_ = BLACK;
        n = &((*n)->children_[idx]);
        (*n)->tag_ = RED;
        sibling = (*n)->children_[!idx];
      } else if(color(sibling->l_) + color(sibling->r_) == BLACK) {
        //all blacks...pormote double-balck to current node
        sibling->tag_ = RED;
        continue;
      }
      if(color(sibling->l_) + color(sibling->r_) == BLACK) {
        //happy ending, no rotation
        (*n)->tag_ = BLACK;
        sibling->tag_ = RED;
        goto REPAIR_DONE;
      }
      auto clr = color(*n);
      (*n)->tag_ = BLACK;
      if(color(sibling->children_[!idx]) == BLACK) {
        if(!idx) promote_rl(*n);
        else promote_lr(*n);
      } else {
        sibling->children_[!idx]->tag_ = BLACK;
        rotate_to_dir(*n,idx);
        //if(!idx) rotate_to_left(*n);
        //else rotate_to_right(*n);
      }
      (*n)->tag_ = clr;
      break;
    }


REPAIR_DONE:
    delete to_delete;
    return true;
  }
  vector<type> collect() const {
    vector<type> ret;
    collect(root_,ret);
    return ret;
  }
  bool verify() const  {
    //rule 1 : root is black
    if(color(root_) != BLACK) {
      cout<<"root is not black" <<endl;
      return false;
    }
    if(!verify_no_rr(root_)) {
      cout<<"there is consecutive red nodes"<<endl;
      return false;
    }
    int cnt = 0;
    const node* n = root_;
    while(n != sink() ) {
      if( color(n) == BLACK) ++cnt;
      n = n->l_;
    }
    if(!verify_black_depth(root_,cnt)) {
      cout<<"inconsistent black depth"<<endl;
      return false;
    }
    return true;
  }
  private:

  //static node* sink() {static node n(type(),NULL,NULL,BLACK); return &n;}
  static node* sink() {
    static const char arr[sizeof(node)] = {};
    return (node*) arr;
  }

  //flag declaration

  const static tag_type BLACK = 0;
  const static tag_type RED = 1;


  const static int FAIL = -1;
  const static int NO_NEED_FIX = 0;
  const static int FIX_SLASH = 1;
  const static int FIX_BACKSLASH = 2;
  const static int CHECK_RR = 3;
  const static int FIX_DOUBLE_BLACK = 4;

  node *root_;
  static tag_type color(node const * const n) {
    return n->tag_;
  }
  static void rotate_to_dir(node*& n, int idx) {
#ifdef RB_PROFILE_ROATATION
    idx ? ++rcnt() : ++lcnt();
#endif
    node* tmp = n->children_[!idx];
    n->children_[!idx] = tmp->children_[idx];
    tmp->children_[idx] = n;
    n = tmp;
  }
  static void rotate_to_right(node*& n) {
#ifdef RB_PROFILE_ROATATION
    ++rcnt();
#endif
    node* tmp = n->l_;
    n->l_ = tmp->r_;
    tmp->r_ = n;
    n = tmp;
  }
  static void rotate_to_left(node*& n) {
#ifdef RB_PROFILE_ROATATION
    ++lcnt();
#endif
    node* tmp = n->r_;
    n->r_ = tmp->l_;
    tmp->l_ = n;
    n = tmp;
  }
  static void promote_lr(node*& n) {
#ifdef RB_PROFILE_ROATATION
    ++lcnt();
    ++rcnt();
#endif
    node* tmp = n->l_->r_;
    n->l_->r_ = tmp->l_;
    tmp->l_ = n->l_;
    n->l_ = tmp->r_;
    tmp->r_ = n;
    n = tmp;
  }
  static void promote_rl(node*& n) {
#ifdef RB_PROFILE_ROATATION
    ++lcnt();
    ++rcnt();
#endif
    node* tmp = n->r_->l_;
    n->r_->l_ = tmp->r_;
    tmp->r_ = n->r_;
    n->r_ = tmp->l_;
    tmp->l_ = n;
    n = tmp;
  }
  static void promote_red(node* n) {
    n->tag_ = RED;
    n->l_->tag_ = n->r_->tag_ = BLACK;
    //n->l_->tag_ = BLACK;
    //n->r_->tag_ = BLACK;
  }
  void collect(node* n, vector<type>& result) const {
    if(n == sink())return;
    collect(n->l_,result);
    result.push_back(n->data_);
    collect(n->r_,result);
  }
  static bool verify_no_rr(node const * const n) {
    if(n == sink())return true;
    if(color(n) == RED && (color(n->l_) == RED || color(n->r_) == RED)) {
      return false;
    }
    return verify_no_rr(n->l_) && verify_no_rr(n->r_);
  }
  static bool verify_black_depth(node const * const n, int d) {
    if(n == sink())return d==0;
    if(color(n) == BLACK) --d;
    return verify_black_depth(n->l_,d) && verify_black_depth(n->r_,d);
  }
};

template<typename type>
class bidir_rb_tree {
  typedef unsigned short tag_type;
  typedef bidir_tagged_node<type, tag_type> node;
  public:

  static int& lcnt() {static int x = 0;return x;}
  static int& rcnt() {static int x = 0;return x;}
  bidir_rb_tree() : root_(sink()) {
#ifdef RB_PROFILE_ROATATION
    lcnt() = rcnt() = 0;
#endif
  }
  bool insert(const type& t) {
    if( root_ == sink()) {
      root_ = new node(t,sink(),sink(),NULL,BLACK);
      return true;
    }
    node* n = root_, **to_new;
    while(true) {
      if(n->data_ < t) {
        if(n->r_ == sink()) {
          //n->r_ = new node(t,sink(),sink(),n,RED);
          //n = n->r_;
          to_new = &n->r_;
          break;
        }
        else {
          n = n->r_;
        }
      } else if( t < n->data_ ) {
        if(n->l_ == sink()) {
          //n->l_ = new node(t,sink(),sink(),n,RED);
          //n = n->l_;
          to_new = &n->l_;
          break;
        }
        else {
          n = n->l_;
        }
      } else return false;
    }
    n = *to_new = new node(t,sink(),sink(),n,RED);
    insert_fix(n);
    return true;
  }
  bool erase(const type& t) {
    node* n = root_;
    while(n != sink()) {
      if(n->data_ < t) {
        n = n->r_;
      } else if( t < n->data_ ) {
        n = n->l_;
      } else {
        erase(n);
        return true;
      }
    }
    return false;
  }
  vector<type> collect() const {
    vector<type> ret;
    collect(root_,ret);
    return ret;
  }
  bool verify() const {
    if(color(root_) != BLACK) {
      cout<<"root is not black" <<endl;
      return false;
    } else if(!verify_no_rr(root_)) {
      cout<<"there is consecutive red nodes"<<endl;
      return false;
    } else if(!verify_linkage(root_)) {
      cout<<"child getting lost"<<endl;
      return false;
    }
    int cnt = 0;
    const node* n = root_;
    while(n != sink() ) {
      if( color(n) == BLACK) ++cnt;
      n = n->l_;
    }
    if(!verify_black_depth(root_,cnt)) {
      cout<<"inconsistent black depth"<<endl;
      return false;
    }
    return true;
  }
  private:

  //empirically using sink outperforms using NULL by 6%
  //static node* sink() {static node n(type(),NULL,NULL,NULL,BLACK); return &n;}
  static node* sink() {
    static char arr [ sizeof(node) ] = {};
    return (node*) arr;
  }
  const static tag_type BLACK = 0;
  const static tag_type RED = 1;
  node *root_;
  static tag_type color(node const * const n) {
    return  n->tag_;
  }
  void rotate_to_right(node* n) {
#ifdef RB_PROFILE_ROATATION
    ++rcnt();
#endif
    node* tmp = n->l_;
    n->l_ = tmp->r_;
    tmp->r_ = n;
    replace_by(n,tmp);
    n->l_->p_ = n;
    n->p_ = tmp;

  }
  void rotate_to_left(node* n) {
#ifdef RB_PROFILE_ROATATION
    ++lcnt();
#endif
    node* tmp = n->r_;
    n->r_ = tmp->l_;
    tmp->l_ = n;
    replace_by(n,tmp);
    n->r_->p_ = n;
    n->p_ = tmp;

  }
  void promote_rl(node* n) {
#ifdef RB_PROFILE_ROATATION
    ++lcnt();
    ++rcnt();
#endif
    node* tmp = n->r_->l_;
    n->r_->l_ = tmp->r_;
    tmp->r_->p_ = n->r_;
    tmp->r_ = n->r_;
    tmp->r_->p_ = tmp;
    n->r_ = tmp->l_;
    n->r_->p_ = n;
    tmp->l_ = n;
    replace_by(n,tmp);
    n->p_ = tmp;
  }
  void promote_lr(node* n) {
#ifdef RB_PROFILE_ROATATION
    ++lcnt();
    ++rcnt();
#endif
    node* tmp = n->l_->r_;
    n->l_->r_ = tmp->l_;
    tmp->l_->p_ = n->l_;
    tmp->l_ = n->l_;
    tmp->l_->p_ = tmp;
    n->l_ = tmp->r_;
    n->l_->p_ = n;
    tmp->r_ = n;
    replace_by(n,tmp);
    n->p_ = tmp;
  }
  void insert_fix_first(node* n) {
    node* p = n->p_;
    if(color(p) == BLACK) return;
    insert_fix_hi(p);
  }
  void insert_fix_hi(node* n) {
    node* p = n->p_;
    if(!p) {
      n->tag_ = BLACK;
      return;
    }
    __builtin_prefetch(p->p_,1,0);
    p->tag_ = RED;
    if(color(p->l_) != color(p->r_)) {
      if(n == p->l_) {
        if(color(n->l_) == BLACK) {
          promote_lr(p);
        } else {
          rotate_to_right(p);
        }
      } else {
        if(color(n->r_) == BLACK) {
          promote_rl(p);
        } else {
          rotate_to_left(p);
        }
      }
      p->p_->tag_ = BLACK;
      return;
    }
    p->l_->tag_ = p->r_->tag_ = BLACK;
    return insert_fix_lo(p);
  }
  void insert_fix_lo(node* n) {
    node* p = n->p_;
    if(p == NULL) {
      n->tag_ = BLACK;
      return;
    }
    if(color(p) == BLACK) return;
    insert_fix_hi(p);
  }
  void insert_fix(node* n) {
    return insert_fix_first(n);
#if 1
    while(n->p_ != NULL) {
      node* parent = n->p_;
      if(color(parent) == BLACK) return;
      node* grand_parent = parent->p_;
      if(grand_parent == NULL) {
        parent->tag_ = BLACK;
        return;
      }
      if(grand_parent->r_ == parent) {
        if(color(grand_parent->l_) == RED) {
          grand_parent->l_->tag_ = parent->tag_ = BLACK;
          grand_parent->tag_ = RED;
        } else {
          if(parent->l_ == n) {
            promote_rl(grand_parent);
            n->tag_ = BLACK;
          } else {
            rotate_to_left(grand_parent);
            parent->tag_ = BLACK;
          }
          grand_parent->tag_ = RED;
          return;
        }
      } else {
        if(color(grand_parent->r_) == RED) {
          grand_parent->r_->tag_ = parent->tag_ = BLACK;
          grand_parent->tag_ = RED;
        } else {
          if(parent->r_ == n) {
            promote_lr(grand_parent);
            n->tag_ = BLACK;
          } else {
            rotate_to_right(grand_parent); 
            parent->tag_ = BLACK;
          }
          grand_parent->tag_ = RED;
          return;
        }
      }
      n = grand_parent;
    }
#else
    while(n->p_ != NULL) {
      node* parent = n->p_;
      if(color(parent) == BLACK) return;
      node* grand_parent = parent->p_;
      if(grand_parent == NULL) {
        parent->tag_ = BLACK;
        return;
      }
      if(grand_parent->r_ == parent) {
        if(color(grand_parent->l_) == RED) {
          grand_parent->l_->tag_ = parent->tag_ = BLACK;
          grand_parent->tag_ = RED;
        } else {
          if(parent->l_ == n) {
            promote_rl(grand_parent);
            parent->tag_ = BLACK;
          } else {
            rotate_to_left(grand_parent);
            n->tag_ = BLACK;
          }
          grand_parent = grand_parent->p_;
        }
      } else {
        if(color(grand_parent->r_) == RED) {
          grand_parent->r_->tag_ = parent->tag_ = BLACK;
          grand_parent->tag_ = RED;
        } else {
          if(parent->r_ == n) {
            promote_lr(grand_parent);
            parent->tag_ = BLACK;
          } else {
            rotate_to_right(grand_parent); 
            n->tag_ = BLACK;
          }
          grand_parent = grand_parent->p_;
        }
      }
      n = grand_parent;
    }
#endif
    root_->tag_ = BLACK;
  }

  void replace_by(node* n, node* n2) {
    if(n==root_) root_ = n2;
    else if(n->p_->l_ == n) n->p_->l_ = n2;
    else n->p_->r_ = n2;
    n2->p_ = n->p_;
  }
  void erase(node* n) {
    node* to_delete = n;
    if(n->l_ == n->r_) {
      replace_by(n,sink());
      if(color(to_delete) ==RED) {
        goto REPAIR_DONE;
      }
      n = n->p_;
    } else if(n->l_ == sink() ) {
      replace_by(n,n->r_);
      n->r_->tag_ = BLACK;
      goto REPAIR_DONE;
    } else if(n->r_ == sink() ) {
      replace_by(n,n->l_);
      n->l_->tag_ = BLACK;
      goto REPAIR_DONE;
    } else if(n->l_->r_ == sink()) {
      n = n->l_;
      auto clr = color(n);
      n->r_ = to_delete->r_;
      n->r_->p_ = n;
      n->tag_ = color(to_delete);
      replace_by(to_delete,n);
      if(n->l_ != sink()) {
        n->l_->tag_ = BLACK;
        goto REPAIR_DONE;
      }
      if(clr !=BLACK) {
        goto REPAIR_DONE;
      }
    } else {
      n = n->l_->r_;
      while(n->r_ != sink()) {
        n = n->r_;
      }
      auto clr = color(n);
      node* orphan = n->l_;
      node* p = n->p_;
      n->l_ = to_delete->l_;
      n->r_ = to_delete->r_;
      n->l_->p_ = n->r_->p_ = n;
      n->tag_ = to_delete->tag_;
      replace_by(to_delete,n);
      p->r_ = orphan;
      if(orphan != sink()) {
        orphan->p_ = p;
        orphan->tag_ = BLACK;
        goto REPAIR_DONE;
      } else if(clr == RED) {
        goto REPAIR_DONE;
      }
      n = p;
    }
    erase_fix(n);
REPAIR_DONE:
    delete to_delete;
  }
  void erase_fix(node* n) {
    node* db= sink();
    while(n) {
      int idx = n->r_ == db;
      node* sibling = n->children_[!idx];
      if(color(n) == RED) {
      } else if(color(sibling) == RED) {
        n->tag_ = RED;
        sibling->tag_ = BLACK;
        if(!idx) rotate_to_left(n);
        else rotate_to_right(n);
        sibling = n->children_[!idx];
      } else if(color(sibling->l_) + color(sibling->r_) == BLACK) {
        sibling->tag_ = RED;
        db = n;
        n = n->p_;
        continue;
      }
      if(color(sibling) == RED) {
        cout<<"fucking disaster"<<endl;
      }
      if(color(sibling->l_) + color(sibling->r_) == BLACK) {
        sibling->tag_ = RED;
        n->tag_ = BLACK;
        return;
      }
      auto clr = color(n);
      n->tag_ = BLACK;
      if(color( sibling->children_[!idx] ) == BLACK) { // zig-zag
        if(!idx) promote_rl(n);
        else promote_lr(n);
      } else {
        sibling->children_[!idx]->tag_ = BLACK;
        if(!idx) rotate_to_left(n);
        else rotate_to_right(n);
      }
      n->p_->tag_ = clr;
      return;
    }
  }
  static void collect(node* n, vector<type>& result) {
    if(n == sink())return;
    collect(n->l_,result);
    result.push_back(n->data_);
    collect(n->r_,result);
  }
  static bool verify_no_rr(node const * const n) {
    if(n == sink())return true;
    if(color(n) == RED && (color(n->l_) == RED || color(n->r_) == RED)) {
      return false;
    }
    return verify_no_rr(n->l_) && verify_no_rr(n->r_);
  }
  static bool verify_black_depth(node const * const n, int d) {
    if(n == sink())return d==0;
    if(color(n) == BLACK) --d;
    return verify_black_depth(n->l_,d) && verify_black_depth(n->r_,d);
  }
  static bool verify_linkage(node const* const n) {
    if(n == sink())return true;
    if(n->l_ != sink() && n->l_->p_ !=n) return false;
    if(n->r_ != sink() && n->r_->p_ !=n) return false;
    return verify_linkage(n->l_) && verify_linkage(n->r_);
  }
};
}//namespace leo
