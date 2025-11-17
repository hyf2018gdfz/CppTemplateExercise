#include "Vector.hpp"

#include <initializer_list>
#include <utility>

namespace mystd::stack {
template <typename T> class Stack {
private:
  vector::Vector<T> m_data;

public:
  Stack() = default;
  Stack(std::initializer_list<T> init) : m_data(init) {}

  bool empty() const { return m_data.empty(); }
  size_t size() const { return m_data.size(); }

  const T &top() const {
    if (empty())
      throw std::out_of_range("Stack::top on empty stack");
    return m_data[0];
  }

  template <typename U> void push(U &&val) {
    m_data.push_back(std::forward(val));
  }

  void pop() {
    if (empty())
      throw std::out_of_range("Stack::pop on empty stack");
    m_data.pop_back();
  }
};
} // namespace mystd::stack