#ifndef COMMON_H
#define COMMON_H

#include <utility>

namespace mystd {
// NOLINTBEGIN(readability-identifier-naming, readability-identifier-length)
template <typename T>
void swap(T &a, T &b) noexcept {
  T temp = std::move(a);
  a = std::move(b);
  b = std::move(temp);
}

template <typename T>
auto max(const T &a, const T &b) noexcept -> T {
  return (a < b ? b : a);
}

template <typename T>
auto min(const T &a, const T &b) noexcept -> T {
  return (a < b ? a : b);
}
// NOLINTEND(readability-identifier-naming, readability-identifier-length)
}  // namespace mystd

#endif  // COMMON_H