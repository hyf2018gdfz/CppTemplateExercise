#ifndef COMPARE_HPP
#define COMPARE_HPP

namespace mystd::compare {

template <typename T>
struct Less {
  constexpr auto operator()(const T &param_a, const T &param_b) const -> bool {
    return param_a < param_b;
  }
};

template <typename T>
struct Greater {
  constexpr auto operator()(const T &param_a, const T &param_b) const -> bool {
    return param_a > param_b;
  }
};
}  // namespace mystd::compare

#endif  // COMPARE_HPP