/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <nlehmann@dcc.uchile.cl> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Nicolás Lehmann
 * ----------------------------------------------------------------------------
 *
 */

#include <bits/basic_k2tree_builder.h>

namespace libk2tree {

template<class _Obj>
basic_k2tree_builder<_Obj>::basic_k2tree_builder(_Obj cnt,
                                                 int k1, int k2, int kl,
                                                 int k1_levels):
    cnt_(cnt),
    size_(0),
    k1_(k1),
    k2_(k2),
    kl_(kl),
    max_level_k1_(k1_levels-1),
    height_(-1),
    leafs_(0),
    edges_(0),
    internal_nodes_(0),  // we dont consider the root
    root() {
  // we extend the size of the matrix to be the multiplication of the arities
  // in all levels (section 5.1). There are k1_levels levels with
  // arity k1, one with arity kl and we must find the numbers of levels with
  // arity k2, ie, the smallest integer x that satisfies:
  // k1^k1_levels * k2^x * kl >= cnt.
  int powk1 = Pow(k1, k1_levels);
  int x = LogCeil(cnt*1.0/powk1/kl, k2);
  if (x == 0)
    fprintf(stderr, "Warning: Ignoring levels with arity k2.\n");

  height_ = k1_levels + x + 1;
  size_ = powk1 * Pow(k2, x) * kl;
}

template<class _Obj>
void basic_k2tree_builder<_Obj>::AddLink(_Obj p, _Obj q) {
  if (root == NULL)
    root = CreateNode(0);
  Node *n = root;
  _Obj N = size_, div_level;
  int child;
  for (int level = 0; level < height_ - 1; level++) {
    int k = level <= max_level_k1_ ? k1_ : k2_;
    div_level = N/k;

    child = p/div_level * k + q/div_level;

    if (n->children_[child] == NULL)
        n->children_[child] = CreateNode(level + 1);

    n = n->children_[child];
    N = div_level, p %= div_level, q %= div_level;
  }
  // n is a node on the level height_ - 1. In this level
  // we store the children information in a BitArray (the leafs)
  div_level = N/kl_;
  child = p/div_level*kl_ + q/div_level;
  if (!n->data_->GetBit(child))
    edges_++;
  n->data_->SetBit(child);
}

template<class _Obj>
shared_ptr<basic_k2tree<_Obj>> basic_k2tree_builder<_Obj>::Build() const {
  BitArray<unsigned int> T(internal_nodes_);
  BitArray<unsigned int> L(leafs_);
  queue<Node*> q;
  q.push(root);

  size_t cnt_level;
  int level;

  // Position on the bitmap T
  size_t pos = -1;
  for (level = 0; level < height_-1; ++level) {
    int k = level <= max_level_k1_ ? k1_ : k2_;
    cnt_level = q.size();
    for (size_t i = 0; i < cnt_level; ++i, ++pos) {
      Node *n = q.front(); q.pop();
      if (n != NULL) {
        if (level > 0)  // if not the root
          T.SetBit(pos);

        for (int child = 0; child < k*k; ++child)
          q.push(n->children_[child]);
      }
    }
  }

  // Visiting nodes on level height - 1
  size_t leaf_pos = 0;
  cnt_level = q.size();
  for (size_t i = 0; i < cnt_level; ++i, ++pos) {
    Node *n = q.front(); q.pop();
    if (n != NULL) {
      T.SetBit(pos);
      for (int child = 0; child < kl_*kl_; ++child, ++leaf_pos)
        if (n->data_->GetBit(child))
          L.SetBit(leaf_pos);
    }
  }

  basic_k2tree<_Obj> *tree = new basic_k2tree<_Obj>(T, L, k1_, k2_, kl_,
                                                    max_level_k1_,
                                                    height_, size_);
  return shared_ptr<basic_k2tree<_Obj> >(tree);
}


template<class _Obj>
basic_k2tree_builder<_Obj>::~basic_k2tree_builder() {
  DeleteNode(root, 0);
}

template<class _Obj>
typename basic_k2tree_builder<_Obj>::Node
*basic_k2tree_builder<_Obj>::CreateNode(int level) {
  basic_k2tree_builder<_Obj>::Node *n = new basic_k2tree_builder<_Obj>::Node();
  if (level < height_ - 1) {
    int k = level <= max_level_k1_ ? k1_ : k2_;
    n->children_ = new Node*[k*k];
    for (int i = 0; i < k*k; ++i)
      n->children_[i] = NULL;
    internal_nodes_ += k*k;
  } else {
    n->data_ = new BitArray<unsigned char>(kl_*kl_);
    leafs_ += kl_*kl_;
  }
  return n;
}

template<class _Obj>
void basic_k2tree_builder<_Obj>::DeleteNode(basic_k2tree_builder<_Obj>::Node *n,
                                            int level) {
  if (n == NULL)
    return;

  if (level < height_ - 1) {
    int k = level <= max_level_k1_ ? k1_ : k2_;
    for (int i = 0; i < k*k; ++i)
      DeleteNode(n->children_[i], level+1);
    --internal_nodes_;
    delete [] n->children_;
  } else {
    leafs_ -= kl_*kl_;
    delete n->data_;
  }
  delete n;
}

template class basic_k2tree_builder<unsigned int>;

}  // namespace libk2tree