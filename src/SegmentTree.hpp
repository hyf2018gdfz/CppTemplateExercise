#ifndef SEGMENTTREE_HPP
#define SEGMENTTREE_HPP

#include <stdexcept>
#include <vector>

#include "BitOperation.hpp"

namespace mystd::segment_tree {

template <typename T, T (*op)(T, T), T (*e)(), typename F, T (*mapping)(F, T),
          F (*composition)(F, F), F (*id)()>
class SegmentTree {
private:
  int arr_size_, tree_size_, height_log_;
  // 为了方便在算法竞赛环境中使用，使用 std::vector 而不是 mystd::Vector
  std::vector<T> tree_;
  std::vector<F> lazy_;
  void pushUp(int ind) { tree_[ind] = op(tree_[ind * 2], tree_[ind * 2 + 1]); }
  void applyNode(int ind, F func) {
    tree_[ind] = mapping(func, tree_[ind]);
    if (ind < tree_size_) {
      lazy_[ind] = composition(func, lazy_[ind]);
    }
  }
  void pushDown(int ind) {
    applyNode(ind * 2, lazy_[ind]);
    applyNode(ind * 2 + 1, lazy_[ind]);
    lazy_[ind] = id();
  }

public:
  explicit SegmentTree(int arr_size)
      : SegmentTree(std::vector<T>(arr_size, e())) {}
  // 数组下标从 0 开始
  explicit SegmentTree(const std::vector<T> &arr)
      : arr_size_(static_cast<int>(arr.size())),
        tree_size_(bitop::bitCeil(arr_size_)),
        height_log_(bitop::countrZero(tree_size_)) {
    tree_.resize(tree_size_ * 2, e());
    lazy_.resize(tree_size_, id());
    for (int i = 0; i < arr_size_; i++) {
      tree_[tree_size_ + i] = arr[i];
    }
    for (int i = tree_size_ - 1; i > 0; i--) {
      pushUp(i);
    }
  }
  void assign(int ind, T val) {
    if (ind < 0 || ind >= arr_size_) {
      throw std::out_of_range("SegmentTree::assign index out of range");
    }
    ind += tree_size_;
    for (int i = height_log_; i >= 1; i--) {
      pushDown(ind >> i);
    }
    tree_[ind] = val;
    for (int i = 1; i <= height_log_; i++) {
      pushUp(ind >> i);
    }
  }
  void apply(int ind, F func) {
    if (ind < 0 || ind >= arr_size_) {
      throw std::out_of_range("SegmentTree::apply index out of range");
    }
    ind += tree_size_;
    for (int i = height_log_; i >= 1; i--) {
      pushDown(ind >> i);
    }
    tree_[ind] = mapping(func, tree_[ind]);
    for (int i = 1; i <= height_log_; i++) {
      pushUp(ind >> i);
    }
  }
  // 区间为闭区间 [L, R]，但内部实现的时候使用半开区间 [L, R+1)，下同
  void apply(int left_bound, int right_bound, F func) {
    if (left_bound < 0 || right_bound >= arr_size_ ||
        left_bound > right_bound) {
      throw std::out_of_range("SegmentTree::apply invalid interval");
    }
    left_bound += tree_size_;
    right_bound += tree_size_ + 1;
    for (int i = height_log_; i >= 1; i--) {
      if (((left_bound >> i) << i) != left_bound) {
        pushDown(left_bound >> i);
      }
      if (((right_bound >> i) << i) != right_bound) {
        pushDown((right_bound - 1) >> i);
      }
    }
    int orig_left = left_bound;
    int orig_right = right_bound;
    while (left_bound < right_bound) {
      if ((left_bound & 1) != 0) {
        applyNode(left_bound++, func);
      }
      if ((right_bound & 1) != 0) {
        applyNode(--right_bound, func);
      }
      left_bound >>= 1;
      right_bound >>= 1;
    }
    for (int i = 1; i <= height_log_; i++) {
      if (((orig_left >> i) << i) != orig_left) {
        pushUp(orig_left >> i);
      }
      if (((orig_right >> i) << i) != orig_right) {
        pushUp((orig_right - 1) >> i);
      }
    }
  }
  auto query(int ind) -> T {
    if (ind < 0 || ind >= arr_size_) {
      throw std::out_of_range("SegmentTree::query index out of range");
    }
    ind += tree_size_;
    for (int i = height_log_; i >= 1; i--) {
      pushDown(ind >> i);
    }
    return tree_[ind];
  }
  auto query(int left_bound, int right_bound) -> T {
    if (left_bound < 0 || right_bound >= arr_size_ ||
        left_bound > right_bound) {
      throw std::out_of_range("SegmentTree::query invalid interval");
    }
    left_bound += tree_size_;
    right_bound += tree_size_ + 1;
    for (int i = height_log_; i >= 1; i--) {
      if (((left_bound >> i) << i) != left_bound) {
        pushDown(left_bound >> i);
      }
      if (((right_bound >> i) << i) != right_bound) {
        pushDown((right_bound - 1) >> i);
      }
    }
    T lans = e();
    T rans = e();
    while (left_bound < right_bound) {
      if ((left_bound & 1) != 0) {
        lans = op(lans, tree_[left_bound++]);
      }
      if ((right_bound & 1) != 0) {
        rans = op(tree_[--right_bound], rans);
      }
      left_bound >>= 1;
      right_bound >>= 1;
    }
    return op(lans, rans);
  }
};
}  // namespace mystd::segment_tree

#endif  // SEGMENTTREE_HPP