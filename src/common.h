#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <sstream>
#include <utility>
#include <stdexcept>

const int iinf = 0x3f3f3f3f;
const long long linf = 2e18;
const int mod = 998244353;

using ll = long long;
using ull = unsigned long long;

template <typename T, typename U, typename = void>
struct is_equality_comparable : std::false_type {};

template <typename T, typename U>
struct is_equality_comparable<
    T, U, std::void_t<decltype(std::declval<T>() == std::declval<U>())>>
    : std::true_type {};

template <typename T, typename U>
constexpr bool is_equality_comparable_v = is_equality_comparable<T, U>::value;

template <typename T, typename = void>
struct is_streamable : std::false_type {};

template <typename T>
struct is_streamable<T, std::void_t<decltype(std::declval<std::ostream &>()
                                             << std::declval<T>())>>
    : std::true_type {};

template <typename T>
constexpr bool is_streamable_v = is_streamable<T>::value;

struct TestInfo {
    const char *fileName;
    const char *funcName;
    int lineNumber;
};

template <typename T, typename U>
void check_eq_impl(const T &expected, const U &actual, const TestInfo &info) {
    bool equal = false;

    if constexpr (is_equality_comparable_v<T, U>) {
        equal = (expected == actual);
    } else {
        // 没法比较，直接比较地址（或强制失败）
        equal = &expected == &actual;
    }

    if (!equal) {
        std::cerr << "[FAIL] " << info.funcName << " (" << info.fileName << ":"
                  << info.lineNumber << "): expected=";

        if constexpr (is_streamable_v<T>) {
            std::cerr << expected;
        } else {
            std::cerr << "<unprintable@" << (const void *)&expected << ">";
        }

        std::cerr << " got=";
        if constexpr (is_streamable_v<U>) {
            std::cerr << actual;
        } else {
            std::cerr << "<unprintable@" << (const void *)&actual << ">";
        }

        std::cerr << "\n";
        throw std::runtime_error("Test failed");
    }
}

#define CHECK_EQ(expected, actual)                                             \
    check_eq_impl((expected), (actual),                                        \
                  (TestInfo){__FILE__, __FUNCTION__, __LINE__})

#endif // COMMON_H