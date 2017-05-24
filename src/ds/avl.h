#pragma once
#include <cstddef>
#include <vector>
#include <iostream>
#include "./tree_node.h"
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

    int sz = 0;
    node** n = &root_;
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

    while(--sz >= 0) {
      node* n = *(traces[sz]);
      int h = height(n->children_[ dirs[sz] ]);
      int diff = h - height(n->children_[ !dirs[sz] ]);
      if(diff == 0) return true; // most frequently hit branch
      if(diff == 1) {
        ++n->tag_;
        continue;
      }
      node*& nref = *(traces[sz]); // useless optimization : put refernce as late as possible
      if(dirs[sz]) {
          if(!dirs[sz+1]) rotate_to_right(nref->r_);
          rotate_to_left(nref);
      } else {
          if(dirs[sz+1]) rotate_to_left(nref->l_);
          rotate_to_right(nref);
      }
      return true;
    }

    return true;
    //easy implementation...20% slower
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
  //empirically using sink outperforms using NULL by 6%
  static node* sink() {static node n(type(),NULL,NULL,0); return &n;}
  const static int FAIL = -1;
  const static int NO_NEED_FIX = 0;
  const static int NEED_FIX = 0;
  node *root_;
  static tag_type height(node const * const n) {
    return  n->tag_;
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
    //cout<<n<<' '<<n->r_<<endl;
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
    if(a<b)a = b;
    n->tag_ = a+1;
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
        replace_byLeftMost(n->r_->l_,n);
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
  static void replace_byLeftMost(node*& n, node*& top) {
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
    replace_byLeftMost(n->l_,top);
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

  //empirically using sink outperforms using NULL by 6%
  static node* sink() {static node n(type(),NULL,NULL,NULL,0); return &n;}
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
    while(n->p_ != NULL) {
      int h = height(n);
      node* parent = n->p_;
      int ph = height(parent);

      if(h < ph)return;
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
    if(a<b)a = b;
    n->tag_ = a+1;
  }
  void replace_by(node* n, node* n2) {
    if(n==root_) root_ = n2;
    else if(n->p_->l_ == n) n->p_->l_ = n2;
    else n->p_->r_ = n2;
    n2->p_ = n->p_;
  }
  void erase(node* n) {
    node* to_fix = n->p_;
    if(n->l_ == sink()) {
      replace_by(n,n->r_);
    } else if(n->r_ == sink()) {
      replace_by(n,n->l_);
    } else {
      node* n2 = n->r_;
      while(n2->l_ != sink()) n2 = n2->l_;
      if(n2->p_ == n) {
        n2->l_ = n->l_;
        n2->l_->p_ = n2;
        n2->tag_ = n->tag_;
        replace_by(n,n2);
        to_fix = n2;
      } else {
        replace_by(n2,n2->r_);
        to_fix = n2->p_;
        n2->l_ = n->l_;
        n2->r_ = n->r_;
        n2->tag_ = n->tag_;
        n->l_->p_ = n->r_->p_ = n2;
        replace_by(n,n2);
      }
    }
    erase_fix(to_fix);
    delete n;
  }
  void erase_fix(node* n) {
    while(n != NULL) {
      int h = height(n);
      int a = height(n->l_);
      int b = height(n->r_);
      int MM = a > b ? a : b;
      if(h!= MM+1) {
        --n->tag_;
        n = n->p_;
        continue;
      }
      if (a > b+1) {
        //if(height(n->l_->r_) > height(n->l_->l_)) rotate_to_left(n->l_);
        //rotate_to_right(n);
        if(height(n->l_->r_) > height(n->l_->l_)) promote_lr(n);
        else rotate_to_right(n);
      } else if(b > a+1) {
        if(height(n->r_->l_) > height(n->r_->r_)) promote_rl(n);
        else rotate_to_left(n);
      } else {
        return;
      }
      n = n->p_;
      if(h ==height(n)) return;
      n = n->p_;
    }
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
