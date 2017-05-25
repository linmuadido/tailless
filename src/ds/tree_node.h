#pragma once


namespace leo {
  template<typename type>
    struct simple_node {
      simple_node() : data_(), l_(), r_() {}
      simple_node(const type& t) : data_(t), l_(), r_() {}
      simple_node(const type& t, simple_node* l, simple_node* r) : data_(t), l_(l), r_(r) {}
      type data_;
      union {
        simple_node* children_[2];
        uintptr_t    ptr_vals_[2];
        struct { simple_node *l_, *r_; };
      };
    };
  template<typename type>
    struct bidir_node {
      bidir_node() : data_(), l_(), r_() {}
      bidir_node(const type& t) : data_(t), l_(), r_(), p_() {}
      bidir_node(const type& t, bidir_node* l, bidir_node* r, bidir_node* p) : data_(t), l_(l), r_(r), p_(p) {}
      type data_;
      bidir_node *l_, *r_, *p_;
    };
  template<typename type, typename tag_type>
    struct tagged_node {
      tagged_node() : data_(), l_(), r_() {}
      tagged_node(const type& t) : data_(t), l_(), r_() {}
      tagged_node(const type& t, tagged_node* l, tagged_node* r) : data_(t), l_(l), r_(r) {}
      tagged_node(const type& t, tagged_node* l, tagged_node* r, tag_type tag) : data_(t), l_(l), r_(r), tag_(tag) {}
      type data_;
      union {
        tagged_node* children_[2];
        struct { tagged_node *l_, *r_; };
      };
      tag_type tag_;
    };
  template<typename type, typename tag_type>
    struct bidir_tagged_node {
      bidir_tagged_node() : data_(), l_(), r_() {}
      bidir_tagged_node(const type& t) : data_(t), l_(), r_() {}
      bidir_tagged_node(const type& t, bidir_tagged_node* l, bidir_tagged_node* r, bidir_tagged_node* p) : data_(t), l_(l), r_(r), p_(),tag_() {}
      bidir_tagged_node(const type& t, bidir_tagged_node* l, bidir_tagged_node* r, bidir_tagged_node* p, tag_type tag) : data_(t), l_(l), r_(r), p_(p), tag_(tag) {}
      type data_;
      bidir_tagged_node *l_, *r_, *p_;
      tag_type tag_;
    };
}
