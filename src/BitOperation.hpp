#ifndef BITOPERATION_HPP
#define BITOPERATION_HPP

#include "common.h"

namespace mystd::bitop {

using ull = unsigned long long;
ull bit_ceil(ull n) {
    ull x = 1;
    while (x < n) x <<= 1;
    return x;
}
ull countr_zero(ull n) {
    ull x = 0;
    while (!(n & (1ull << x))) x++;
    return x;
}
} // namespace mystd::bitop

#endif // BITOPERATION_HPP