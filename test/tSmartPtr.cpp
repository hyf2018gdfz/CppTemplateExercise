#include <type_traits>

#include "SmartPtr.hpp"
#include "test.h"

using mystd::memory::UniquePtr;

namespace TestSmartPtr {
// --- 辅助测试类：追踪资源泄漏 ---
struct Tracker {
  static int alive_count;
  int value;
  Tracker(int v = 0) : value(v) { alive_count++; }
  virtual ~Tracker() { alive_count--; }
};
int Tracker::alive_count = 0;

struct DerivedTracker : Tracker {
  DerivedTracker(int v = 0) : Tracker(v) {}
  ~DerivedTracker() override = default;
};

// --- 辅助测试类：各类删除器 ---
struct TrueEmptyDeleter {
  void operator()(Tracker* p) const { delete p; }
};
struct CallCountingDeleter {
  int* call_count = nullptr;
  void operator()(Tracker* p) const {
    if (call_count) (*call_count)++;
    delete p;
  }
};
struct StatefulDeleter {
  int state;
  void operator()(Tracker* p) const { delete p; }
};
};  // namespace TestSmartPtr

using namespace TestSmartPtr;

static void test_unique_ptr_basic() {
  // 1. 默认构造与 nullptr 构造
  UniquePtr<int> p1;
  UniquePtr<int> p2(nullptr);
  CHECK_EQ(true, p1.get() == nullptr);
  CHECK_EQ(true, p2.get() == nullptr);
  CHECK_EQ(false, static_cast<bool>(p1));

  // 2. 原始指针构造与解引用
  UniquePtr<int> p3(new int(42));
  CHECK_EQ(true, p3.get() != nullptr);
  CHECK_EQ(true, static_cast<bool>(p3));
  CHECK_EQ(42, *p3);

  // 3. 修改所指对象
  *p3 = 100;
  CHECK_EQ(100, *p3);

  // 4. operator->
  UniquePtr<Tracker> p4(new Tracker(99));
  CHECK_EQ(99, p4->value);
}

static void test_unique_ptr_reset_release() {
  Tracker::alive_count = 0;  // 重置计数器

  {
    UniquePtr<Tracker> p1(new Tracker(10));
    CHECK_EQ(1, Tracker::alive_count);

    // 1. 测试 release
    Tracker* raw = p1.release();
    CHECK_EQ(true, p1.get() == nullptr);
    CHECK_EQ(1, Tracker::alive_count);  // 释放控制权，不应析构

    // 2. 测试带参数的 reset
    p1.reset(raw);  // 重新接管
    CHECK_EQ(1, Tracker::alive_count);
    CHECK_EQ(10, p1->value);

    // 替换为新指针，旧资源必须被销毁
    p1.reset(new Tracker(20));
    CHECK_EQ(1, Tracker::alive_count);
    CHECK_EQ(20, p1->value);

    // 3. 测试无参 reset (等价于赋 nullptr)
    p1.reset();
    CHECK_EQ(0, Tracker::alive_count);
    CHECK_EQ(true, p1.get() == nullptr);

    // 4. 测试 nullptr 赋值
    p1.reset(new Tracker(30));
    CHECK_EQ(1, Tracker::alive_count);
    p1 = nullptr;
    CHECK_EQ(0, Tracker::alive_count);
  }
  // 离开作用域，确保清零
  CHECK_EQ(0, Tracker::alive_count);
}

static void test_unique_ptr_move() {
  Tracker::alive_count = 0;
  {
    // 1. 同类型移动构造
    UniquePtr<Tracker> p1(new Tracker(1));
    UniquePtr<Tracker> p2(std::move(p1));
    CHECK_EQ(true, p1.get() == nullptr);
    CHECK_EQ(1, p2->value);
    CHECK_EQ(1, Tracker::alive_count);

    // 2. 同类型移动赋值
    UniquePtr<Tracker> p3(new Tracker(3));
    p3 = std::move(p2);
    CHECK_EQ(true, p2.get() == nullptr);
    CHECK_EQ(1, p3->value);
    CHECK_EQ(1, Tracker::alive_count);
  }
  CHECK_EQ(0, Tracker::alive_count);
  {
    // 3. 跨类型移动构造 (Derived -> Base)
    UniquePtr<DerivedTracker> p_derived(new DerivedTracker(100));
    UniquePtr<Tracker> p_base(std::move(p_derived));
    CHECK_EQ(true, p_derived.get() == nullptr);
    CHECK_EQ(100, p_base->value);

    // 4. 跨类型移动赋值
    UniquePtr<DerivedTracker> p_derived2(new DerivedTracker(200));
    p_base = std::move(p_derived2);
    CHECK_EQ(true, p_derived2.get() == nullptr);
    CHECK_EQ(200, p_base->value);

    // 期望总共只有 p_base 持有 1 个 Tracker
    CHECK_EQ(1, Tracker::alive_count);
    p_base.reset();
    CHECK_EQ(0, Tracker::alive_count);
  }
}

static void test_unique_ptr_deleters() {
  // 1. 验证 EBCO (空基类优化)
  // 使用 TrueEmptyDeleter，EBCO 必定生效，UniquePtr 大小应等于单指针大小
  // (8字节)
  CHECK_EQ(sizeof(void*), sizeof(UniquePtr<Tracker, TrueEmptyDeleter>));

  // 带有状态的删除器，体积必然大于单个指针 (16字节)
  CHECK_EQ(true, sizeof(UniquePtr<Tracker, StatefulDeleter>) > sizeof(void*));

  // 2. 验证左值引用删除器 (Deleter&)
  int calls = 0;
  CallCountingDeleter del{&calls};
  {
    // 传入引用，底层保存指针和引用，总大小应为 16 字节 (sizeof(void*)*2)
    UniquePtr<Tracker, CallCountingDeleter&> p1(new Tracker(5), del);
    CHECK_EQ(sizeof(void*) * 2, sizeof(p1));
  }
  // p1 析构后，必须通过我们的 del 实例调用，calls 应变为 1
  CHECK_EQ(1, calls);

  {
    CallCountingDeleter del2{&calls};
    UniquePtr<Tracker, CallCountingDeleter&> p2(new Tracker(6), del2);
    UniquePtr<Tracker, CallCountingDeleter&> p3(std::move(p2));
    CHECK_EQ(true, p2.get() == nullptr);
  }
  // 移动后销毁，应该只发生一次析构
  CHECK_EQ(2, calls);

  // 3. 验证值类型删除器的移动转移
  {
    StatefulDeleter s_del{99};
    UniquePtr<Tracker, StatefulDeleter> p4(new Tracker(7), s_del);
    CHECK_EQ(99, p4.getDeleter().state);

    UniquePtr<Tracker, StatefulDeleter> p5(std::move(p4));
    CHECK_EQ(99, p5.getDeleter().state);
  }
}

MAKE_TEST(UniquePtr, Basic) { test_unique_ptr_basic(); }
MAKE_TEST(UniquePtr, ResetRelease) { test_unique_ptr_reset_release(); }
MAKE_TEST(UniquePtr, MoveSemantics) { test_unique_ptr_move(); }
MAKE_TEST(UniquePtr, DeletersAndEBCO) { test_unique_ptr_deleters(); }