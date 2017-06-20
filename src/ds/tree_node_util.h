#pragma once


namespace leo {
namespace tree_node_util {

template< typename node >
inline void rotate_to_dir(node*& n, int idx) {
  node* tmp = n->children_[!idx];
  n->children_[!idx] = tmp->children_[idx];
  tmp->children_[idx] = n;
  n = tmp;
}
template< typename node >
inline void rotate_to_left_ref(node*& n) {
  return rotate_to_dir(n,0);
}
template< typename node >
inline void rotate_to_right_ref(node*& n) {
  return rotate_to_dir(n,1);
}
template< typename node >
inline void promote_zz_ref(node*& n,int idx) {
  node* z = n->children_[idx];
  node* zz = z->children_[!idx];
  n->children_[idx] = zz->children_[!idx];
  z->children_[!idx] = zz->children_[idx];
  zz->children_[!idx] = n;
  zz->children_[idx] = z;
  n = zz;
}
template < typename node >
inline void promote_rl_ref(node*& n) {
  promote_zz_ref(n,1);
}
template < typename node >
inline void promote_lr_ref(node*& n) {
  promote_zz_ref(n,0);
}


} // namespace tree_node_util
} // namespace leo
