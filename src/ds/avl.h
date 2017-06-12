#pragma once
#include <cstddef>
#include <vector>
#include <iostream>
#include "./tree_node.h"
#include "helper/compiler.h"
#include "helper/debug.h"
using namespace std;


#define AVL_PROFILE_ROATATION
#define AVL_PROFILE_INSERTION_BRANCH

namespace leo {
template<typename type>
class avl {
  typedef unsigned short tag_type;
  typedef tagged_node<type, tag_type> node;
  public:

  static int& lcnt() {static int x = 0;return x;}
  static int& rcnt() {static int x = 0;return x;}
  avl() : root_(sink()) {
#ifdef AVL_PROFILE_ROATATION
    lcnt() = rcnt() = 0;
#endif
  }
  bool insert(const type& t) {

    node**        traces[128];
    unsigned char dirs[128];
    node** n = &root_;
    int sz = 0;

    while(*n != sink()) {
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
    }
    *n = new node(t,sink(),sink(),1);
    if(--sz >= 0) {
      node* n = *(traces[sz]);
      if(height(n) == 2) return true;
      n->tag_ += 1;
    }
    int h = 2;
    while(--sz >= 0) {
      node* n = *(traces[sz]);
      int h_sibling = height(n->children_[ !dirs[sz] ] );
      int diff = (h++) - h_sibling;
      if(diff == 0) return true; // most frequently hit branch
      if(diff == 1) {
        ++n->tag_;
        continue;
      }
      node*& nref = *(traces[sz]); // useless optimization : put refernce as late as possible
      if(dirs[sz]) {
          if(!dirs[sz+1]) promote_rl(nref);
          else rotate_to_left_zagzag(nref);
      } else {
          if(dirs[sz+1]) promote_lr(nref);
          else rotate_to_right_zigzig(nref);
      }
      return true;
    }
    return true;
    //easy implementation...20% slower
    return insert(root_,t);
  }
  bool erase(const type& t) {
    //return erase(root_,t);

    //if(root_ == sink())return false;

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
    if(!sz && height(root_) == 1) {
      delete root_;
      root_ = sink();
      return true;
    }

    node* to_delete = *n;
    node* prev = sink();
    switch( height(*n) ) {
      case 1 : {
        *n = sink();
        n = traces[--sz];
        if( (*n)->children_[ !dirs[sz] ] == sink() ) {
          --(*n)->tag_;
          prev = *n;
        } else if(height((*n)->children_[ !dirs[sz] ]) == 2) {
          ++sz;
        } else {
          goto REPAIR_DONE;
        }
      } break;
      case 2 : {
        int idx = (*n)->l_ == sink() ? 1 : 0;
        node* n2 = (*n)->children_[!idx];
        *n = (*n)->children_[idx];
        (*n)->children_[!idx] = n2;
        prev = *n;
        if(n2 == sink()) break;
        (*n)->tag_ = 2;
        goto REPAIR_DONE;
      }break;
      default: {
        node** to_be_altered = n;
        traces[sz] = n;
        dirs[sz++] = 0;
        n = &((*n)->l_);
        int subtree_pos = sz;
        while((*n)->r_ != sink()) {
          traces[sz] = n;
          dirs[sz++] = 1;
          n = &((*n)->r_);
        }
        node* to_replace = *n;
        node* to_promote = to_replace->l_;
        *n = to_promote;
        prev = to_promote;
        to_replace->l_ = (*to_be_altered)->l_;
        to_replace->r_ = (*to_be_altered)->r_;
        to_replace->tag_ = (*to_be_altered)->tag_;
        *to_be_altered = to_replace;
        traces[subtree_pos] = &to_replace->l_;
      }
    }
    while(--sz >= 0) {
      node** n = traces[sz];
      int idx = dirs[sz];
      //node* n_short = (*n)->children_[  idx  ];
      node* n_short = prev;
      node* n_tall =  (*n)->children_[ !idx  ];
      if( height(n_tall) == height(n_short)+1 ) goto REPAIR_DONE;
      if( height(n_tall) == height(n_short)   ) (*n)->tag_ -= 1;
      else {
        int diff = height(n_tall->children_[!idx]) - height(n_tall->children_[idx]);
        if(diff>=0) {
          if(!idx) {
            rotate_to_left(*n);
          } else {
            rotate_to_right(*n);
          }
          if(!diff)break;
        } else {
          if(!idx) {
            promote_rl(*n);
          } else {
            promote_lr(*n);
          }
        }
      }
      prev = *n;
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
  bool verify() const {
    if(verify_height(root_) == false) {
      cout<<"broken height definition."<<endl;
      return false;
    } else if(verify_balance(root_) == false) {
      cout<<"leaning AVL"<<endl;
      return false;
    }
    return true;
  }
  private:
  //the following typical way introduce static magic (i.e. synchrization)
  //static node* sink() {static node n(type(),NULL,NULL,0); return &n;}
  //the following version can reduce the total instruction counts by 20%
  //however stalled cycles increase, still need to find out the reason 
  static node* sink() {
    const static char arr[sizeof(node)] = {};
    return (node*)arr;
  }
  const static int FAIL = -1;
  const static int NO_NEED_FIX = 0;
  const static int NEED_FIX = 0;
  node *root_;
  static tag_type height(node const * const n) {
    return  n->tag_;
  }
  static void rotate_to_right_zigzig(node*& n) {
#ifdef AVL_PROFILE_ROATATION
    ++rcnt();
#endif
    --n->tag_;
    node* tmp = n->l_;
    n->l_ = tmp->r_;
    tmp->r_ = n;
    n = tmp;
  }
  static void rotate_to_left_zagzag(node*& n) {
#ifdef AVL_PROFILE_ROATATION
    ++lcnt();
#endif
    --n->tag_;
    node* tmp = n->r_;
    n->r_ = tmp->l_;
    tmp->l_ = n;
    n = tmp;
  }
  static void promote_rl(node*& n) {
    node* r = n->r_;
    node* rl = r->l_;
    n->tag_ = r->tag_ = rl->tag_++;
    n->r_ = rl->l_;
    r->l_ = rl->r_;
    rl->l_ = n;
    rl->r_ = r;
    n = rl;
  }
  static void promote_lr(node*& n) {
    node* l = n->l_;
    node* lr = l->r_;
    n->tag_ = l->tag_ = lr->tag_++;
    n->l_ = lr->r_;
    l->r_ = lr->l_;
    lr->l_ = n;
    lr->r_ = l;
    n = lr;
  }
  static void rotate_to_right(node*& n) {
#ifdef AVL_PROFILE_ROATATION
    ++rcnt();
#endif
    node* tmp = n->l_;
    n->l_ = tmp->r_;
    tmp->r_ = n;
    n = tmp;
    update_height(n->r_);
    n->tag_ = n->r_->tag_+1;
  }
  static void rotate_to_left(node*& n) {
#ifdef AVL_PROFILE_ROATATION
    ++lcnt();
#endif
    node* tmp = n->r_;
    n->r_ = tmp->l_;
    tmp->l_ = n;
    n = tmp;
    update_height(n->l_);
    n->tag_ = n->l_->tag_+1;
  }
  static void update_height(node* n) {
    int a = height(n->l_);
    int b = height(n->r_);
    n->tag_ = (a+b+3)>>1;
  }
  static bool insert(node*& n, const type& t) {
    if( n == sink() ) {
      n = new node(t,sink(),sink(),1);
      return true;
    }
    if( n->data_ == t) return false;
    //if(!insert(n->data_ < t ? n->r_ : n->l_, t)) return false;
    if( n->data_ <  t) {
      if(!insert(n->r_,t))return false;
      if(height(n->r_) > height(n->l_) +1 ) {
        if(height(n->r_->l_) > height(n->r_->r_)) rotate_to_right(n->r_);
        rotate_to_left(n);
      } else update_height(n);
    } else {
      if(!insert(n->l_,t))return false;
      if(height(n->l_) > height(n->r_) +1 ) {
        if(height(n->l_->r_) > height(n->l_->l_)) rotate_to_left(n->l_);
        rotate_to_right(n);
      } else update_height(n);
    }
    //update_height(n);
    return true;
  }
  bool erase(node*& n, const type& t) {
    if( n == sink() ) return false;
    if( n->data_ == t) {

      node* toDelete = n;
      if(n->l_ == sink()) {
        n = n->r_;
        if(n != sink()) n->tag_ = height(n->r_) + 1;
      }
      else if(n->r_ == sink()) {
        n = n->l_;
        if(n!= sink()) n->tag_ = height(n->l_) + 1;
      }
      else if(n->r_->l_ == sink()) {
        n->r_->l_ = n->l_;
        n = n->r_;
        update_height(n);
      } else if(n->l_->r_ == sink()) {
        n->l_->r_ = n->r_;
        n = n->l_;
        update_height(n);
      } else {
        replace_by_leftmost(n->r_->l_,n);
        check_n_fix(n->r_);
      }
      check_n_fix(n);
      delete toDelete;
      return true;
    }
    if(n->data_ < t) {
      if(!erase(n->r_,t)) return false;
      if(height(n->l_) > height(n->r_) +1) {
        if(height(n->l_->r_) > height(n->l_->l_)) rotate_to_left(n->l_);
        rotate_to_right(n);
      } else {
        update_height(n);
      }
    } else {
      if(!erase(n->l_,t)) return false;
      if(height(n->r_) > height(n->l_) +1 ) {
        if(height(n->r_->l_) > height(n->r_->r_)) rotate_to_right(n->r_);
        rotate_to_left(n);
      } else {
        update_height(n);
      }
    }
      //check_n_fix(n);
    return true;
  }
  static void check_n_fix(node*& n) {
    if(n==sink())return;
    update_height(n);
    int a = height(n->l_);
    int b = height(n->r_);
    if(a-b >1) {
      if(height(n->l_->r_) > height(n->l_->l_)) rotate_to_left(n->l_);
      rotate_to_right(n);
    } else if( b-a >1) {
      if(height(n->r_->l_) > height(n->r_->r_)) rotate_to_right(n->r_);
      rotate_to_left(n);
    }
  }
  static void replace_by_leftmost(node*& n, node*& top) {
    //cout<<n->data_<<endl;
    if(n->l_ == sink()) {
      //cout<<"reaching end: right is ";
      //if(n->r_ == NULL) cout<<"(NULL)"<<endl;
      //else cout<<n->r_->data_<<endl;
      node* tmp = top;
      top = n;
      n = n->r_;
      top->l_ = tmp->l_;
      top->r_ = tmp->r_;
      top->tag_ = tmp->tag_;
      return;
    }
    //cout<<"keeping going: right is ";
    //if(n->r_ == NULL) cout<<"(NULL)"<<endl;
    //else cout<<n->r_->data_<<endl;
    replace_by_leftmost(n->l_,top);
    check_n_fix(n);
  }
  static void collect(node* n, vector<type>& result) {
    if(n == sink())return;
    collect(n->l_,result);
    result.push_back(n->data_);
    collect(n->r_,result);
  }
  static bool verify_height(node const* const n) {
    if(n == sink())return true;
    int a = height(n->l_);
    int b = height(n->r_);
    if(a<b)a=b;
    if(height(n) != a+1)return false;
    return verify_height(n->l_) && verify_height(n->r_);
  }
  static bool verify_balance(node const* const n) {
    if(n == sink())return true;
    int a = height(n->l_);
    int b = height(n->r_);
    if(a-b>1 || b-a>1) {
      return false;
    }
    return verify_balance(n->l_) && verify_balance(n->r_);
  }
};
template<typename type>
class bidir_avl {
  typedef unsigned short tag_type;
  typedef bidir_tagged_node<type, tag_type> node;
  public:

  static int& lcnt() {static int x = 0;return x;}
  static int& rcnt() {static int x = 0;return x;}
  bidir_avl() : root_(sink()) {
#ifdef AVL_PROFILE_ROATATION
    lcnt() = rcnt() = 0;
#endif
  }
  bool insert(const type& t) {
    if( root_ == sink()) {
      root_ = new node(t,sink(),sink(),NULL,1);
      return true;
    }
    node* n = root_;
    while(true) {
      if(n->data_ < t) {
        if(n->r_ == sink()) {
          n->r_ = new node(t,sink(),sink(),n,1);
          n = n->r_;
          break;
        }
        else {
          n = n->r_;
        }
      } else if( t < n->data_ ) {
        if(n->l_ == sink()) {
          n->l_ = new node(t,sink(),sink(),n,1);
          n = n->l_;
          break;
        }
        else {
          n = n->l_;
        }
      } else return false;
    }
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
    if(verify_linkage(root_) == false) {
      cout<<"child getting lost"<<endl;
      return false;
    } else if(verify_height(root_) == false) {
      cout<<"broken height definition."<<endl;
      return false;
    } else if(verify_balance(root_) == false) {
      cout<<"leaning AVL"<<endl;
      return false;
    }
    return true;
  }
  private:

  
  //static node* sink() {static node n(type(),NULL,NULL,NULL,0); return &n;}
  //the following hack avoid static magic, however more stalled cycles in insertion fix
  static node* sink() {
    static char arr[sizeof(node)] = {};
    return  (node*) arr;
  }
  /* tried this version, almost as good as static one
  char arr_[sizeof(node)] = {};
  node* sink() {
    return  (node*) arr_;
  }
  */
  const static int FAIL = -1;
  const static int NO_NEED_FIX = 0;
  const static int NEED_FIX = 0;
  node *root_;
  static tag_type height(node const * const n) {
    return  n->tag_;
  }
  void rotate_to_right(node* n) {
#ifdef AVL_PROFILE_ROATATION
    ++rcnt();
#endif
    node* tmp = n->l_;
    n->l_ = tmp->r_;
    tmp->r_ = n;
    replace_by(n,tmp);
    n->p_ = tmp;
    n->l_->p_ = n;

    update_height(n);
    tmp->tag_ = n->tag_+1;
  }
  void rotate_to_left(node* n) {
#ifdef AVL_PROFILE_ROATATION
    ++lcnt();
#endif
    node* tmp = n->r_;
    n->r_ = tmp->l_;
    tmp->l_ = n;
    replace_by(n,tmp);
    n->p_ = tmp;
    n->r_->p_ = n;

    update_height(n);
    tmp->tag_ = n->tag_+1;
  }
  void rotate_to_dir(node* n,int idx) {
#ifdef AVL_PROFILE_ROATATION
    idx ? ++rcnt() : ++lcnt();
#endif
    node* tmp = n->children_[!idx];
    n->tag_ = height(tmp->children_[idx])+1;
    tmp->tag_ = height(n)+1;
    n->children_[!idx] = tmp->children_[idx];
    tmp->children_[idx] = n;
    replace_by(n,tmp);
    n->p_ = tmp;
    n->children_[!idx]->p_ = n;

    update_height(n);
    tmp->tag_ = n->tag_+1;
  }
  void rotate_to(node* n, int idx) {
    if(idx) ++rcnt();
    else ++lcnt();

    node* tmp = n->children_[idx];
    n->children_[idx] = tmp->children_[!idx];
    tmp->children_[idx] = n;
    replace_by(n,tmp);
    n->p_ = tmp;
    n->children_[!idx]->p_ = n;
    update_height(n);
    tmp->tag_ = n->tag_+1;
  }
  void promote_zz(node* n, int idx) {
#ifdef AVL_PROFILE_ROATATION
    ++lcnt();
    ++rcnt();
#endif
    node* tmp = n->children_[idx]->children_[!idx];
    n->children_[idx]->children_[!idx] = tmp->children_[idx];
    tmp->children_[idx]->p_ = n->children_[idx];
    tmp->children_[idx] = n->children_[idx];
    tmp->children_[idx]->p_ = tmp;
    n->children_[idx] = tmp->children_[!idx];
    n->children_[idx]->p_ = n;
    tmp->children_[!idx] = n;
    replace_by(n,tmp);
    n->p_ = tmp;
    n->tag_ = tmp->tag_;
    --tmp->children_[idx]->tag_;
    ++tmp->tag_;
  }
  void promote_rl(node* n) {
#ifdef AVL_PROFILE_ROATATION
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
    n->tag_ = tmp->tag_;
    --tmp->r_->tag_;
    ++tmp->tag_;
  }
  void promote_lr(node* n) {
#ifdef AVL_PROFILE_ROATATION
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
    n->tag_ = tmp->tag_;
    --tmp->l_->tag_;
    ++tmp->tag_;
  }
  void insert_fix(node* n) {
    __builtin_prefetch(sink(), 1,0);
    if(height(n->p_) == 2) return;
    n = n->p_;
    n->tag_ = 2;
    while(n->p_ != NULL) {
      int h = height(n);
      node* parent = n->p_;
      int ph = height(parent);

      if(h < ph) return;
      if(parent->r_ == n) {
        if(height(parent->l_) == h-1) ++parent->tag_;
        else {
          if(height(n->l_) > height(n->r_)) promote_rl(parent);
          else rotate_to_left(parent);
          return;
        }
      } else {
        if(height(parent->r_) == h-1) ++parent->tag_;
        else {
          if(height(n->r_) > height(n->l_)) promote_lr(parent);
          else rotate_to_right(parent);
          return;
        }
      }
      n = parent;
    }
  }

  static void update_height(node* n) {
    int a = height(n->l_);
    int b = height(n->r_);
    n->tag_ = (a+b+3)>>1;
  }
  void replace_by(node* n, node* n2) {
    node* p = n->p_;
    if(!p) {
      root_ = n2;
    }
    else p->children_[n == p->r_] = n2;
    //else if(n->p_->l_ == n) n->p_->l_ = n2;
    //else n->p_->r_ = n2;
    n2->p_ = n->p_;
  }
  void erase(node* n) {
    //cout<<height(n)<<endl;
    node* to_delete = n;
    switch( height(n) ) {
      case 1: {
        node* p = n->p_;
        if(!p) { // root, the only node in the tree
          root_ = sink();
          goto REPAIR_DONE;
        }
        p->children_[n == p->r_] = sink();
        n = p;
      } break;
      case 2: {
        int idx = n->l_ == sink();
        node* to_promote = n->children_[idx];
        node* orphan = n->children_[!idx];
        //adopt the orphan
        to_promote->children_[!idx] = orphan;
        orphan->p_ = to_promote;
        //TODO: partial replacement
        node* p = n->p_;
        if(!p) {
          root_ = to_promote;
          if(orphan != sink()) {
            to_promote->tag_ = 2;
          }
          goto REPAIR_DONE;
        }
        p->children_[p->l_ != n] = to_promote;
        to_promote->p_ = p;
        if(orphan != sink()) {
          to_promote->tag_ = 2;
          goto REPAIR_DONE;
        }
        n = p;
      } break;
      default: {
        int idx = height(n->l_) < height(n->r_);
        node* n2 = n->children_[idx];
        if(n2->children_[!idx] == sink()) { // partial replacement
          node* p = n->p_;
          p->children_[p->r_ == n] = n2;
          n2->p_ = p;
          n2->children_[!idx] = n->children_[!idx];
          n2->children_[!idx]->p_ = n2;
          n2->tag_ = n->tag_;
          if(n2->children_[idx] == sink()) {
          } else if( height(n2->children_[!idx]) == 2) {
            goto REPAIR_DONE;
          } else {
          }
          n = n2;
          break;
        }
        do {
          n2 = n2->children_[!idx];
        }while(n2->children_[!idx] != sink());
        node* orphan = n2->children_[idx];
        node* p = n2->p_;
        p->children_[!idx] = orphan;
        orphan->p_ = p;
        replace_by(n,n2);
        n2->children_[idx] = n->children_[idx];
        n2->children_[!idx] = n->children_[!idx];
        n->children_[idx]->p_ = n->children_[!idx]->p_ = n2; 
        n2->tag_ = n->tag_;
        n = p;
      } break;
    }
    do {
      node* l = n->l_;
      node* r = n->r_;
      int h_l = height(l);
      int h_r = height(r);
      if( (h_l^h_r) & 1 ) goto REPAIR_DONE;
      node* p = n->p_;
      if(h_l == h_r) {
        --n->tag_;
      } else {
        int idx = h_r > h_l;
        node* n_tall = n->children_[idx];
        int diff = height(n_tall->children_[idx]) - height(n_tall->children_[!idx]);
        if(diff>=0) {
          rotate_to_dir(n,!idx);
          //if(idx) rotate_to_left(n);
          //else rotate_to_right(n);
          if(!diff) goto REPAIR_DONE;
        }
        else {
          //promote_zz(n,idx);
          if(idx) promote_rl(n);
          else promote_lr(n);
        }
      }
      n = p;
    } while( n );
REPAIR_DONE:
    delete to_delete;
  }
  inline node* sibling( node* p, node* n) {
    return (node*)(p->ptr_vals_[0] ^ p->ptgr_vals_[1] ^ uintptr_t(n));
    return n== p->l_ ? p->r_ : p->l_;
    return p->children_[p->l_ == n];
  }
  static void collect(node* n, vector<type>& result) {
    if(n == sink())return;
    collect(n->l_,result);
    result.push_back(n->data_);
    collect(n->r_,result);
  }
  static bool verify_height(node const* const n) {
    if(n == sink())return true;
    int a = height(n->l_);
    int b = height(n->r_);
    if(a<b)a=b;
    if(height(n) != a+1)return false;
    return verify_height(n->l_) && verify_height(n->r_);
  }
  static bool verify_balance(node const* const n) {
    if(n == sink())return true;
    int a = height(n->l_);
    int b = height(n->r_);
    if(a-b>1 || b-a>1) {
      return false;
    }
    return verify_balance(n->l_) && verify_balance(n->r_);
  }
  static bool verify_linkage(node const* const n) {
    if(n == sink())return true;
    if(n->l_ != sink() && n->l_->p_ !=n) return false;
    if(n->r_ != sink() && n->r_->p_ !=n) return false;
    return verify_linkage(n->l_) && verify_linkage(n->r_);
  }
};
}//namespace leo
