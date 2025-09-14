#ifndef COMPARE_HPP
#define COMPARE_HPP

#include "common.h"

namespace mystd {

template <typename T>
struct Less {
    constexpr bool operator()(const T &a, const T &b) const { return a < b; }
};

template <typename T>
struct Greater {
    constexpr bool operator()(const T &a, const T &b) const { return a > b; }
};
} // namespace mystd

#endif // COMPARE_HPP