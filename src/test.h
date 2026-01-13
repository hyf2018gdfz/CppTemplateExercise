#ifndef TEST_H
#define TEST_H

#include <climits>
#include <cstddef>
#include <iostream>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <utility>

template <typename T, typename U, typename = void>
struct IsEqualityComparable : std::false_type {};

template <typename T, typename U>
struct IsEqualityComparable<
    T, U, std::void_t<decltype(std::declval<T>() == std::declval<U>())>>
    : std::true_type {};

template <typename T, typename U>
constexpr bool IS_EQUALITY_COMPARABLE_V = IsEqualityComparable<T, U>::value;

template <typename T, typename = void>
struct IsStreamable : std::false_type {};

template <typename T>
struct IsStreamable<T, std::void_t<decltype(std::declval<std::ostream &>()
                                            << std::declval<T>())>>
    : std::true_type {};

template <typename T>
constexpr bool IS_STREAMABLE_V = IsStreamable<T>::value;

struct TestInfo {
  const char *file_name_;
  const char *func_name_;
  int line_number_;
};

template <typename T, typename U>
void checkEqImpl(const T &expected, const U &actual, const TestInfo &info) {
  bool equal = false;

  if constexpr (IS_EQUALITY_COMPARABLE_V<T, U>) {
    equal = (expected == actual);
  } else {
    // 没法比较，直接比较地址（或强制失败）
    equal = &expected == &actual;
  }

  if (!equal) {
    std::cerr << "[FAIL] " << info.func_name_ << " (" << info.file_name_ << ":"
              << info.line_number_ << "): expected=";

    if constexpr (IS_STREAMABLE_V<T>) {
      std::cerr << expected;
    } else {
      std::cerr << "<unprintable@" << (const void *)&expected << ">";
    }

    std::cerr << " got=";
    if constexpr (IS_STREAMABLE_V<U>) {
      std::cerr << actual;
    } else {
      std::cerr << "<unprintable@" << (const void *)&actual << ">";
    }

    std::cerr << "\n";
    throw std::runtime_error("Test failed");
  }
}

#define CHECK_EQ(expected, actual)  \
  checkEqImpl((expected), (actual), \
              (TestInfo){__FILE__, __FUNCTION__, __LINE__})

#define EXPECT_THROW(statement, exception_type) \
  do {                                          \
    bool caught = false;                        \
    try {                                       \
      statement;                                \
    } catch (const exception_type &) {          \
      caught = true;                            \
    }                                           \
    CHECK_EQ(true, caught);                     \
  } while (0)

#define EXPECT_NO_THROW(statement) \
  do {                             \
    bool caught = false;           \
    try {                          \
      statement;                   \
    } catch (...) {                \
      caught = true;               \
    }                              \
    CHECK_EQ(false, caught);       \
  } while (0)

// NOLINTBEGIN
class RandomGenerator {
private:
  std::mt19937 rng_;

public:
  RandomGenerator() : rng_(std::random_device{}()) {}

  explicit RandomGenerator(std::mt19937::result_type seed) : rng_(seed) {}

  auto operator()() -> size_t {
    std::uniform_int_distribution<size_t> dist(0, SIZE_MAX);
    return dist(rng_);
  }

  // 生成指定范围的随机整数
  template <class A, class B>
  auto uniform_int(A min, B max) -> std::common_type_t<A, B> {
    using T = std::common_type_t<A, B>;
    static_assert(std::is_integral_v<T>);
    std::uniform_int_distribution<T> dist(static_cast<T>(min),
                                          static_cast<T>(max));
    return dist(rng_);
  }

  // 生成指定范围的随机浮点数
  template <class A, class B>
  auto uniform_real(A min, B max) -> double {
    static_assert(std::is_arithmetic_v<A> && std::is_arithmetic_v<B>);
    std::uniform_real_distribution<double> dist(static_cast<double>(min),
                                                static_cast<double>(max));
    return dist(rng_);
  }

  // 生成随机布尔值
  auto bernoulli(double p = 0.5) -> bool {
    std::bernoulli_distribution dist(p);
    return dist(rng_);
  }

  auto engine() -> std::mt19937 & { return rng_; }
};
// NOLINTEND

#endif  // TEST_H