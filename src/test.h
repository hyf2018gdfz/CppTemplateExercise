#ifndef TEST_H
#define TEST_H

#include <climits>
#include <cstddef>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
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

#include <map>
#include <vector>

using TestFunc = void (*)();

struct Testcase {
  std::string suite_name_;
  std::string case_name_;
  TestFunc func_;
};

class TestRegistry {
  const std::string RESET = "\033[0m";
  const std::string RED = "\033[31m";
  const std::string GREEN = "\033[32m";
  const std::string CYAN = "\033[36m";

public:
  static auto instance() -> TestRegistry & {
    static TestRegistry reg;
    return reg;
  }
  // 注册函数
  void addTest(const std::string &suite, const std::string &name,
               TestFunc func) {
    Testcase tmp{suite, name, func};
    tests_[suite].push_back(tmp);
  }
  // main 直接调用的执行接口
  auto runAll() -> int {
    int global_failed = 0;
    for (auto &suite : tests_) {
      std::cout << CYAN << "==== Running suite " << suite.first
                << " ====" << RESET << "\n";
      auto total = static_cast<int>(suite.second.size());
      auto passed = 0;
      for (auto &testcase : suite.second) {
        try {
          testcase.func_();
          passed++;
        } catch (std::exception &e) {
          std::cerr << RED << "[ERROR] " << RESET << testcase.case_name_
                    << " failed: " << e.what() << "\n";
        }
      }
      int failed = total - passed;
      global_failed += failed;
      if (passed == total) {
        std::cout << GREEN << "All " << total << " tests passed!" << RESET
                  << "\n";
      } else {
        std::cout << GREEN << "Passed: " << passed << RESET << "\n";
        std::cout << RED << "Failed: " << failed << RESET << "\n";
      }
    }
    return global_failed;
  }

  // 只运行指定套件
  auto runSuites(const std::vector<std::string> &suites) -> int {
    int global_failed = 0;
    for (const auto &name : suites) {
      auto iter = tests_.find(name);
      if (iter == tests_.end()) {
        std::cerr << RED << "[WARN] " << RESET << "Unknown suite: " << name
                  << "\n";
        continue;
      }

      std::cout << CYAN << "==== Running " << iter->first << " ====" << RESET
                << "\n";
      auto total = static_cast<int>(iter->second.size());
      auto passed = 0;
      for (auto &testcase : iter->second) {
        try {
          testcase.func_();
          passed++;
        } catch (std::exception &e) {
          std::cerr << RED << "[ERROR] " << RESET << testcase.case_name_
                    << " failed: " << e.what() << "\n";
        }
      }
      int failed = total - passed;
      global_failed += failed;
      if (passed == total) {
        std::cout << GREEN << "All " << total << " tests passed!" << RESET
                  << "\n";
      } else {
        std::cout << GREEN << "Passed: " << passed << RESET << "\n";
        std::cout << RED << "Failed: " << failed << RESET << "\n";
      }
    }
    return global_failed;
  }

  // 列出所有套件名
  [[nodiscard]] auto getSuites() const -> std::vector<std::string> {
    std::vector<std::string> out;
    out.reserve(tests_.size());
    for (const auto &suite : tests_) {
      out.push_back(suite.first);
    }
    return out;
  }

  // 列出指定套件下的用例名
  [[nodiscard]] auto getCases(const std::string &suite) const
      -> std::vector<std::string> {
    std::vector<std::string> out;
    auto iter = tests_.find(suite);
    if (iter == tests_.end()) {
      return out;
    }
    for (const auto &testcase : iter->second) {
      out.push_back(testcase.case_name_);
    }
    return out;
  }

private:
  std::map<std::string, std::vector<Testcase>> tests_;
};

#define MAKE_TEST(suite_name, case_name)                                \
  void suite_name##_##case_name##_impl();                               \
  static int suite_name##_##case_name##_reg = []() {                    \
    TestRegistry::instance().addTest(#suite_name, #case_name,           \
                                     &suite_name##_##case_name##_impl); \
    return 0;                                                           \
  }();                                                                  \
  void suite_name##_##case_name##_impl()

#endif  // TEST_H