#ifndef COMPARE_HPP
#define COMPARE_HPP

namespace mystd::compare {

template <typename T>
struct Less {
    constexpr bool operator()(const T &a, const T &b) const { return a < b; }
};

template <typename T>
struct Greater {
    constexpr bool operator()(const T &a, const T &b) const { return a > b; }
};
} // namespace mystd::compare

#endif // COMPARE_HPP