#include <type_traits>

#include "SmartPtr.hpp"
#include "test.h"

namespace TestSmartPtr {
struct TestDeleter {
  void operator()(int* p) const { delete p; }
};
};  // namespace TestSmartPtr

static void test_uptr_constructor() {}

MAKE_TEST(UniquePtr, Constructor) { test_uptr_constructor(); }