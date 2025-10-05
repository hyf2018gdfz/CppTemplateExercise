#ifndef TEST_H
#define TEST_H

#include <climits>
#include <cstddef>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

template <typename T, typename U, typename = void>
struct is_equality_comparable : std::false_type {};

template <typename T, typename U>
struct is_equality_comparable<
    T,
    U,
    std::void_t<decltype(std::declval<T>() == std::declval<U>())>>
    : std::true_type {};

template <typename T, typename U>
constexpr bool is_equality_comparable_v = is_equality_comparable<T, U>::value;

template <typename T, typename = void>
struct is_streamable : std::false_type {};

template <typename T>
struct is_streamable<
    T,
    std::void_t<decltype(std::declval<std::ostream &>() << std::declval<T>())>>
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
    check_eq_impl(                                                             \
        (expected), (actual), (TestInfo){__FILE__, __FUNCTION__, __LINE__})

#define EXPECT_THROW(statement, exception_type)                                \
    do {                                                                       \
        bool caught = false;                                                   \
        try {                                                                  \
            statement;                                                         \
        } catch (const exception_type &) { caught = true; }                    \
        CHECK_EQ(true, caught);                                                \
    } while (0)

#define EXPECT_NO_THROW(statement)                                             \
    do {                                                                       \
        bool caught = false;                                                   \
        try {                                                                  \
            statement;                                                         \
        } catch (...) { caught = true; }                                       \
        CHECK_EQ(false, caught);                                               \
    } while (0)

class RandomGenerator {
private:
    std::mt19937 rng_;

public:
    RandomGenerator() : rng_(std::random_device{}()) {}

    explicit RandomGenerator(std::mt19937::result_type seed) : rng_(seed) {}

    size_t operator()() {
        std::uniform_int_distribution<size_t> dist(0, SIZE_MAX);
        return dist(rng_);
    }

    // 生成指定范围的随机整数
    template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    T uniform_int(T min, T max) {
        std::uniform_int_distribution<T> dist(min, max);
        return dist(rng_);
    }

    // 生成指定范围的随机浮点数
    template <typename T,
              std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    T uniform_real(T min, T max) {
        std::uniform_real_distribution<T> dist(min, max);
        return dist(rng_);
    }

    // 生成随机布尔值
    bool bernoulli(double p = 0.5) {
        std::bernoulli_distribution dist(p);
        return dist(rng_);
    }

    std::mt19937 &engine() { return rng_; }
};

#endif // TEST_H