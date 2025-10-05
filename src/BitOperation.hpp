#ifndef BITOPERATION_HPP
#define BITOPERATION_HPP

namespace mystd::bitop {

using ull = unsigned long long;
constexpr ull bit_ceil(ull n) noexcept {
    ull x = 1;
    while (x < n) x <<= 1;
    return x;
}
constexpr ull countr_zero(ull n) noexcept {
    ull x = 0;
    while (!(n & (1ull << x))) x++;
    return x;
}
constexpr ull lowbit(ull n) noexcept {
    return n & -n;
}
} // namespace mystd::bitop

#endif // BITOPERATION_HPP