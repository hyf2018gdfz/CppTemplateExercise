#ifndef SET_HPP
#define SET_HPP

#include <algorithm>
#include <cstddef>
#include <utility>

#include "Compare.hpp"
#include "RBTree.hpp"

namespace mystd::set {

template <typename T, typename Compare = mystd::compare::Less<T>>
class Set {
private:
  struct Identity {
    auto operator()(const T& val) const -> const T& { return val; }
  };
  using RBTree = mystd::rbtree::RBTree<T, T, Identity, Compare>;
  RBTree tree_;

public:
  using Iterator = typename RBTree::ConstIterator;
  using ConstIterator = typename RBTree::ConstIterator;
  Set() = default;
  explicit Set(Compare comp) : tree_(comp) {}
  Set(const Set& other) : tree_(other.tree_) {}
  Set(Set&& other) noexcept : tree_(std::move(other.tree_)) {}
  auto operator=(const Set& other) -> Set& {
    tree_ = other.tree_;
    return *this;
  }
  auto operator=(Set&& other) noexcept -> Set& {
    tree_ = std::move(other);
    return *this;
  }
  ~Set() = default;

  // clang-format off
  auto begin() noexcept -> Iterator { return Iterator(tree_.begin()); }
  auto begin() const noexcept -> ConstIterator { return ConstIterator(tree_.begin()); }
  auto end() -> Iterator { return Iterator(tree_.end()); }
  auto end() const noexcept -> ConstIterator { return ConstIterator(tree_.end()); }
  [[nodiscard]] auto size() const noexcept -> size_t { return tree_.size(); }
  [[nodiscard]] auto empty() const noexcept -> bool { return tree_.empty(); }
  // clang-format on

  auto insert(const T& val) -> std::pair<Iterator, bool> {
    auto tmp = tree_.insertUnique(val);
    return {Iterator(tmp.first), tmp.second};
  }
  auto insert(T&& val) -> std::pair<Iterator, bool> {
    auto tmp = tree_.insertUnique(std::move(val));
    return {Iterator(tmp.first), tmp.second};
  }

  auto erase(Iterator iter) {
    typename RBTree::Iterator tmp(iter.base());
    typename RBTree::Iterator res = tree_.erase(tmp);
    return Iterator(res);
  }
  auto erase(const T& val) { return tree_.eraseUnique(val); }

  // clang-format off
  auto find(const T& val) { return Iterator(tree_.find(val)); }
  auto find(const T& val) const { return ConstIterator(tree_.find(val)); }
  auto lowerBound(const T& val) { return Iterator(tree_.lowerBound(val)); }
  auto lowerBound(const T& val) const { return ConstIterator(tree_.lowerBound(val)); }
  auto upperBound(const T& val) { return Iterator(tree_.upperBound(val)); }
  auto upperBound(const T& val) const { return ConstIterator(tree_.upperBound(val)); }
  auto equalRange(const T& val) -> std::pair<Iterator, Iterator> {
    auto tmp = tree_.equalRange(val);
    return {Iterator(tmp.first), Iterator(tmp.second)};
  }
  auto equalRange(const T& val) const -> std::pair<ConstIterator, ConstIterator> {
    auto tmp = tree_.equalRange(val);
    return {ConstIterator(tmp.first), ConstIterator(tmp.second)};
  }
  // clang-format on

  void clear() { tree_.clear(); }
  void swap(Set& other) noexcept { tree_.swap(other.tree_); }
};

template <typename T, typename Compare = mystd::compare::Less<T>>
class Multiset {
private:
  struct Identity {
    auto operator()(const T& val) const -> const T& { return val; }
  };
  using RBTree = mystd::rbtree::RBTree<T, T, Identity, Compare>;
  RBTree tree_;

public:
  using iterator = typename RBTree::Iterator;
  Multiset() : tree_() {}
  ~Multiset() = default;

  auto begin() -> iterator { return tree_.begin(); }
  auto end() -> iterator { return tree_.end(); }
  auto size() -> size_t { return tree_.size(); }
  auto empty() -> bool { return tree_.empty(); }

  auto insert(const T& val) { return tree_.insert(val); }
  auto insert(T&& val) { return tree_.insert(std::move(val)); }

  auto erase(iterator iter) { return tree_.erase(iter); }
  auto erase(const T& val) { return tree_.eraseMulti(val); }

  auto find(const T& val) { return tree_.find(val); }
  auto lowerBound(const T& val) { return tree_.lowerBound(val); }
  auto upperBound(const T& val) { return tree_.upperBound(val); }

  void clear() { tree_.clear(); }
};

}  // namespace mystd::set

#endif