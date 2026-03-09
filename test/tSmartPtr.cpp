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

// Array版本的各类测试辅助类
struct ArrayTracker {
  static int alive_count;
  int value;

  ArrayTracker() : value(0) { alive_count++; }
  ~ArrayTracker() { alive_count--; }

  ArrayTracker(const ArrayTracker&) = delete;
  ArrayTracker& operator=(const ArrayTracker&) = delete;
};
int ArrayTracker::alive_count = 0;

struct BaseTrackerArray {
  virtual ~BaseTrackerArray() = default;
};
struct DerivedTrackerArray : BaseTrackerArray {};
struct TrueEmptyArrayDeleter {
  void operator()(ArrayTracker* p) const { delete[] p; }
};
struct CallCountingArrayDeleter {
  int* call_count = nullptr;
  void operator()(ArrayTracker* p) const {
    if (call_count) (*call_count)++;
    delete[] p;
  }
};
// --- 辅助测试类：Fancy Pointer (自定义指针) ---
struct MyIntArrayPtr {
  int* p;
  MyIntArrayPtr(int* p = nullptr) : p(p) {}
  MyIntArrayPtr(std::nullptr_t) : p(nullptr) {}
  bool operator==(std::nullptr_t) const { return p == nullptr; }
  bool operator!=(std::nullptr_t) const { return p != nullptr; }
  explicit operator bool() const { return p != nullptr; }
  int& operator[](std::size_t i) const { return p[i]; }
};

struct FancyArrayDeleter {
  using Pointer = MyIntArrayPtr;
  void operator()(MyIntArrayPtr ptr) const { delete[] ptr.p; }
};
struct HasStar {
  template <typename T, typename = decltype(*std::declval<T>())>
  static std::true_type test(int);
  template <typename>
  static std::false_type test(...);
  template <typename UPtr>
  static constexpr bool value = decltype(test<UPtr>(0))::value;
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

static void test_unique_ptr_array_basic() {
  ArrayTracker::alive_count = 0;

  {
    // 1. 默认构造与 nullptr 构造
    UniquePtr<ArrayTracker[]> p1;
    UniquePtr<ArrayTracker[]> p2(nullptr);
    CHECK_EQ(true, p1.get() == nullptr);
    CHECK_EQ(true, p2.get() == nullptr);
    CHECK_EQ(false, static_cast<bool>(p1));

    // 2. 原始数组指针构造
    UniquePtr<ArrayTracker[]> p3(new ArrayTracker[5]);
    CHECK_EQ(true, p3.get() != nullptr);
    CHECK_EQ(true, static_cast<bool>(p3));
    // 必须有 5 个元素存活
    CHECK_EQ(5, ArrayTracker::alive_count);

    // 3. 测试 operator[] 访问与修改
    p3[0].value = 10;
    p3[4].value = 20;
    CHECK_EQ(10, p3[0].value);
    CHECK_EQ(20, p3[4].value);
  }
  // 离开作用域，p3 析构，必须调用 delete[] 销毁 5 个元素
  CHECK_EQ(0, ArrayTracker::alive_count);
}

static void test_unique_ptr_array_reset_release() {
  ArrayTracker::alive_count = 0;

  {
    UniquePtr<ArrayTracker[]> p1(new ArrayTracker[3]);
    CHECK_EQ(3, ArrayTracker::alive_count);

    // 1. release：交出所有权，但不销毁
    ArrayTracker* raw = p1.release();
    CHECK_EQ(true, p1.get() == nullptr);
    CHECK_EQ(3, ArrayTracker::alive_count);  // 仍然是 3

    // 2. reset(raw)：重新接管
    p1.reset(raw);
    CHECK_EQ(3, ArrayTracker::alive_count);

    // 3. reset(new)：销毁旧数组，接管新数组
    p1.reset(new ArrayTracker[2]);
    // 旧的 3 个被销毁，新的 2 个被创建，总存活应该变为 2
    CHECK_EQ(2, ArrayTracker::alive_count);

    // 4. reset() 无参调用：等价于置空
    p1.reset();
    CHECK_EQ(0, ArrayTracker::alive_count);
    CHECK_EQ(true, p1.get() == nullptr);
  }
  CHECK_EQ(0, ArrayTracker::alive_count);
}

static void test_unique_ptr_array_move() {
  ArrayTracker::alive_count = 0;

  {
    UniquePtr<ArrayTracker[]> p1(new ArrayTracker[3]);
    p1[0].value = 100;

    // 1. 同类型移动构造
    UniquePtr<ArrayTracker[]> p2(std::move(p1));
    CHECK_EQ(true, p1.get() == nullptr);
    CHECK_EQ(100, p2[0].value);
    CHECK_EQ(3, ArrayTracker::alive_count);

    // 2. 同类型移动赋值
    UniquePtr<ArrayTracker[]> p3(new ArrayTracker[2]);
    CHECK_EQ(5, ArrayTracker::alive_count);  // p2(3) + p3(2) = 5

    p3 = std::move(p2);
    CHECK_EQ(true, p2.get() == nullptr);
    CHECK_EQ(100, p3[0].value);
    // p3 原本的 2 个元素必须被 delete[]，只剩下接管的 3 个
    CHECK_EQ(3, ArrayTracker::alive_count);

    // 3. 自我移动赋值 (Self-move) 安全性验证
    p3 = std::move(p3);
    CHECK_EQ(100, p3[0].value);              // 数据还在
    CHECK_EQ(3, ArrayTracker::alive_count);  // 没有发生 double free
  }
  CHECK_EQ(0, ArrayTracker::alive_count);
}

static void test_unique_ptr_array_swap() {
  ArrayTracker::alive_count = 0;

  {
    UniquePtr<ArrayTracker[]> p1(new ArrayTracker[2]);
    p1[0].value = 10;

    UniquePtr<ArrayTracker[]> p2(new ArrayTracker[3]);
    p2[0].value = 20;

    // 成员函数 swap
    p1.swap(p2);
    CHECK_EQ(20, p1[0].value);
    CHECK_EQ(10, p2[0].value);
    CHECK_EQ(5, ArrayTracker::alive_count);

    // 全局 std::swap (通常会调用成员函数，或者引发移动构造/赋值)
    mystd::swap(p1, p2);
    CHECK_EQ(10, p1[0].value);
    CHECK_EQ(20, p2[0].value);
    CHECK_EQ(5, ArrayTracker::alive_count);
  }
  CHECK_EQ(0, ArrayTracker::alive_count);
}

static void test_unique_ptr_array_sfinae() {
  using UPtr = mystd::memory::UniquePtr<int[]>;

  // 1. 严格禁用拷贝构造和拷贝赋值
  CHECK_EQ(false, (std::is_copy_constructible_v<UPtr>));
  CHECK_EQ(false, (std::is_copy_assignable_v<UPtr>));

  // 2. 允许安全的 const 转换 (int[] -> const int[])
  using ConstUPtr = mystd::memory::UniquePtr<const int[]>;
  CHECK_EQ(true,
           (std::is_constructible_v<ConstUPtr, UPtr&&>));  // 跨类型移动允许追加
                                                           // const

  // 3. 拦截数组多态 (Derived[] 绝对不能转为 Base[])
  using BaseUPtr = mystd::memory::UniquePtr<BaseTrackerArray[]>;
  using DerivedUPtr = mystd::memory::UniquePtr<DerivedTrackerArray[]>;
  // 3.1 拦截跨类型移动赋值/构造
  CHECK_EQ(false, (std::is_constructible_v<BaseUPtr, DerivedUPtr&&>));
  // 3.2 拦截直接用 Derived* 裸指针构造 BaseUPtr
  CHECK_EQ(false, (std::is_constructible_v<BaseUPtr, DerivedTrackerArray*>));

  // 4. 拦截单元素与数组的混用 (UniquePtr<int> 与 UniquePtr<int[]>)
  using SingleUPtr = mystd::memory::UniquePtr<int>;
  CHECK_EQ(false, (std::is_constructible_v<UPtr, SingleUPtr&&>));
  CHECK_EQ(false, (std::is_constructible_v<SingleUPtr, UPtr&&>));

  // 5. 验证是否禁用了 operator* 和 operator-> (通过 void_t 探测)
  CHECK_EQ(false, HasStar::value<UPtr>);
}

static void test_unique_ptr_array_advanced() {
  // 1. 验证空基类优化 (EBCO) 在数组版本中生效
  CHECK_EQ(
      sizeof(void*),
      sizeof(mystd::memory::UniquePtr<ArrayTracker[], TrueEmptyArrayDeleter>));

  // 2. 验证左值引用删除器 (Deleter&) 与 nullptr_t 的联动
  int calls = 0;
  CallCountingArrayDeleter del{&calls};
  {
    // 测试：用 nullptr 和 引用删除器 初始化
    mystd::memory::UniquePtr<ArrayTracker[], CallCountingArrayDeleter&> p(
        nullptr, del);
    CHECK_EQ(true, p.get() == nullptr);
  }
  // 对 nullptr 进行析构时，不应该触发删除器
  CHECK_EQ(0, calls);

  {
    // 测试：正常资源配合引用删除器
    mystd::memory::UniquePtr<ArrayTracker[], CallCountingArrayDeleter&> p2(
        new ArrayTracker[2], del);
  }
  // 析构正常对象，必须触发一次删除器
  CHECK_EQ(1, calls);

  // 3. 零长度数组测试 (合法但边缘的 C++ 行为)
  ArrayTracker::alive_count = 0;
  {
    // C++ 允许 new T[0]，通常会返回一个非 nullptr
    // 的唯一地址，且析构时行为应正常
    mystd::memory::UniquePtr<ArrayTracker[]> p_zero(new ArrayTracker[0]);
    CHECK_EQ(true, p_zero.get() != nullptr);
  }
  // 没有元素被构造，自然存活数为 0，且 delete[] p_zero 不能崩溃
  CHECK_EQ(0, ArrayTracker::alive_count);

  // 4. Fancy Pointer (自定义指针) SFINAE 回退机制测试
  {
    MyIntArrayPtr fancy(new int[3]);
    mystd::memory::UniquePtr<int[], FancyArrayDeleter> p_fancy(fancy);
    CHECK_EQ(true, static_cast<bool>(p_fancy));

    // reset 一个新的 Fancy Pointer
    p_fancy.reset(MyIntArrayPtr(new int[2]));
    CHECK_EQ(true, static_cast<bool>(p_fancy));
  }  // 离开作用域，FancyArrayDeleter 负责清理
}

// clang-format off
MAKE_TEST(UniquePtr, Basic) { test_unique_ptr_basic(); }
MAKE_TEST(UniquePtr, ResetRelease) { test_unique_ptr_reset_release(); }
MAKE_TEST(UniquePtr, MoveSemantics) { test_unique_ptr_move(); }
MAKE_TEST(UniquePtr, DeletersAndEBCO) { test_unique_ptr_deleters(); }
MAKE_TEST(UniquePtr, Basic_arr) { test_unique_ptr_array_basic(); }
MAKE_TEST(UniquePtr, ResetRelease_arr) { test_unique_ptr_array_reset_release(); }
MAKE_TEST(UniquePtr, MoveSemantics_arr) { test_unique_ptr_array_move(); }
MAKE_TEST(UniquePtr, Swap_arr) { test_unique_ptr_array_swap(); }
MAKE_TEST(UniquePtr, SFINAE_arr) { test_unique_ptr_array_sfinae(); }
MAKE_TEST(UniquePtr, Advanced_arr) { test_unique_ptr_array_advanced(); }