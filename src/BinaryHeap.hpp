#ifndef BINARY_HEAP_HPP
#define BINARY_HEAP_HPP

#include <initializer_list>

#include "Compare.hpp"
#include "Vector.hpp"
#include "common.h"

namespace mystd::binary_heap {

/// INFO: 基于 mystd::Vector 实现的二叉堆，默认大根堆
template <typename T, typename Compare = mystd::compare::Less<T>>
class BinaryHeap {
private:
  vector::Vector<T> data_;
  Compare comp_;

  void siftUp(size_t idx) {
    while (idx > 0) {
      size_t parent = (idx - 1) / 2;
      if (!comp_(data_[parent], data_[idx])) {
        break;
      }
      mystd::swap(data_[parent], data_[idx]);
      idx = parent;
    }
  }

  void siftDown(size_t idx) {
    size_t heap_size = data_.size();
    while (true) {
      size_t left = idx * 2 + 1;
      size_t right = idx * 2 + 2;
      size_t largest = idx;

      if (left < heap_size && comp_(data_[largest], data_[left])) {
        largest = left;
      }
      if (right < heap_size && comp_(data_[largest], data_[right])) {
        largest = right;
      }
      if (largest == idx) {
        break;
      }
      mystd::swap(data_[idx], data_[largest]);
      idx = largest;
    }
  }

public:
  explicit BinaryHeap(const Compare &comp = Compare()) : data_(), comp_(comp) {}

  BinaryHeap(std::initializer_list<T> init, const Compare &comp = Compare())
      : data_(init), comp_(comp) {
    buildHeap();
  }

  [[nodiscard]] auto empty() const -> bool { return data_.empty(); }
  [[nodiscard]] auto size() const -> size_t { return data_.size(); }

  auto top() const -> const T & {
    if (empty()) {
      throw std::out_of_range("BinaryHeap::top on empty heap");
    }
    return data_[0];
  }

  template <typename U>
  void push(U &&value) {
    data_.pushBack(std::forward<U>(value));
    siftUp(data_.size() - 1);
  }

  template <typename... Args>
  void emplace(Args &&...args) {
    data_.emplaceBack(std::forward<Args>(args)...);
    siftUp(data_.size() - 1);
  }

  void pop() {
    if (empty()) {
      throw std::out_of_range("BinaryHeap::pop on empty heap");
    }
    mystd::swap(data_[0], data_[data_.size() - 1]);
    data_.popBack();
    if (!empty()) {
      siftDown(0);
    }
  }

  void swap(BinaryHeap &ano) noexcept {
    this->data_.swap(ano.data_);
    mystd::swap(this->comp_, ano.comp_);
  }

private:
  void buildHeap() {
    if (data_.empty()) {
      return;
    }
    for (size_t i = (data_.size() - 1) / 2 + 1; i > 0; --i) {
      siftDown(i - 1);
    }
  }
};

}  // namespace mystd::binary_heap

#endif  // BINARY_HEAP_HPP
