#ifndef COMMON_H
#define COMMON_H

#include <utility>

namespace mystd {

template <typename T>
void swap(T &a, T &b) noexcept {
    T temp = std::move(a);
    a = std::move(b);
    b = std::move(temp);
}

template <typename T>
T max(const T &a, const T &b) noexcept {
    return (a < b ? b : a);
}

template <typename T>
T min(const T &a, const T &b) noexcept {
    return (a < b ? a : b);
}
} // namespace mystd

#endif // COMMON_H