#ifndef BITOPERATION_HPP
#define BITOPERATION_HPP

namespace mystd::bitop {
// NOLINTBEGIN
using ull = unsigned long long;
constexpr auto bitCeil(ull n) noexcept -> ull {
  ull x = 1;
  while (x < n) {
    x <<= 1;
  }
  return x;
}
constexpr auto countrZero(ull n) noexcept -> ull {
  ull x = 0;
  while ((n & (1ULL << x)) == 0U) {
    x++;
  }
  return x;
}
constexpr auto lowbit(ull n) noexcept -> ull { return n & -n; }
// NOLINTEND
}  // namespace mystd::bitop

#endif  // BITOPERATION_HPP