#ifndef FENWICKTREE_HPP
#define FENWICKTREE_HPP

#include <stdexcept>
#include <vector>

#include "BitOperation.hpp"

namespace mystd::fenwick_tree {

// op: T × T -> T，满足结合律，e 是单位元
// opInv: 两段前缀和做差
template <typename T, T (*e)(), T (*op)(T, T), T (*opInv)(T, T)>
class FenwickTree {
private:
  int arr_size_;
  std::vector<T> tree_;

public:
  explicit FenwickTree(int arr_size)
      : FenwickTree(std::vector<T>(arr_size, e())) {}
  // 数组下标从0开始，但是平移到1开始
  explicit FenwickTree(const std::vector<T> &arr)
      : arr_size_(static_cast<int>(arr.size())), tree_(arr_size_ + 1, e()) {
    for (int i = 1; i <= arr_size_; i++) {
      tree_[i] = op(tree_[i], arr[i - 1]);
      int j = i + static_cast<int>(bitop::lowbit(i));
      if (j <= arr_size_) {
        tree_[j] = op(tree_[j], tree_[i]);
      }
    }
  }

  void apply(int ind, T val) {
    if (ind < 0 || ind >= arr_size_) {
      throw std::out_of_range("FenwickTree::apply invalid index");
    }
    ind++;
    for (; ind <= arr_size_; ind += static_cast<int>(bitop::lowbit(ind))) {
      tree_[ind] = op(tree_[ind], val);
    }
  }
  // 此处求和得到的是前缀和
  auto query(int ind) const -> T {
    if (ind < -1 || ind >= arr_size_) {
      throw std::out_of_range("FenwickTree::query invalid index");
    }
    ind++;
    T result = e();
    for (; ind > 0; ind -= static_cast<int>(bitop::lowbit(ind))) {
      result = op(result, tree_[ind]);
    }
    return result;
  }

  auto query(int left_bound, int right_bound) const -> T {
    if (left_bound < 0 || right_bound >= arr_size_ ||
        left_bound > right_bound) {
      throw std::out_of_range("FenwickTree::query invalid interval");
    }
    return opInv(query(right_bound), query(left_bound - 1));
  }
};
}  // namespace mystd::fenwick_tree

#endif  // FENWICKTREE_HPP