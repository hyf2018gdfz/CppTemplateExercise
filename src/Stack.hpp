#ifndef STACK_HPP
#define STACK_HPP

#include <initializer_list>
#include <utility>

#include "Vector.hpp"

namespace mystd::stack {
template <typename T>
class Stack {
private:
  vector::Vector<T> data_;

public:
  Stack() = default;
  Stack(std::initializer_list<T> init) : data_(init) {}

  [[nodiscard]] auto empty() const -> bool { return data_.empty(); }
  [[nodiscard]] auto size() const -> size_t { return data_.size(); }

  auto top() const -> const T & {
    if (empty()) {
      throw std::out_of_range("Stack::top on empty stack");
    }
    return data_.back();
  }

  template <typename U>
  void push(U &&val) {
    data_.pushBack(std::forward<U>(val));
  }

  void pop() {
    if (empty()) {
      throw std::out_of_range("Stack::pop on empty stack");
    }
    data_.popBack();
  }
};
}  // namespace mystd::stack
#endif