#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <sstream>
#include <utility>
#include <stdexcept>

namespace mystd {

template <typename T>
void swap(T &a, T &b) noexcept {
    T temp = std::move(a);
    a = std::move(b);
    b = std::move(temp);
}
} // namespace mystd

#endif // COMMON_H