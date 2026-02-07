#include <set>
#include <vector>

#include "Compare.hpp"
#include "Set.hpp"
#include "test.h"
using mystd::set::Multiset;
using std::multiset;

static void multiset_construct_impl() {
  {
    Multiset<int> mst;
    CHECK_EQ(0, mst.size());
    CHECK_EQ(true, mst.empty());
    CHECK_EQ(mst.begin(), mst.end());
  }
  {
    Multiset<int, mystd::compare::Greater<int>> mst;
    mst.insert(3);
    mst.insert(1);
    mst.insert(2);
    CHECK_EQ(3, *mst.begin());
    CHECK_EQ(2, *(++mst.begin()));
  }
  {
    Multiset<int> mst1;
    mst1.insert(1);
    mst1.insert(2);
    Multiset<int> mst2 = mst1;
    mst1.insert(3);
    CHECK_EQ(3, mst1.size());
    CHECK_EQ(2, mst2.size());
    CHECK_EQ(mst2.end(), mst2.find(3));
  }
  {
    Multiset<int> mst1;
    mst1.insert(1);
    mst1.insert(2);
    Multiset<int> mst2 = std::move(mst1);
    CHECK_EQ(2, mst2.size());
    CHECK_EQ(true, mst1.empty());
  }
  {
    Multiset<int> mst1, mst2;
    mst1.insert(1);
    mst1.insert(2);
    mst2 = mst1;
    mst1.insert(3);
    CHECK_EQ(3, mst1.size());
    CHECK_EQ(2, mst2.size());
    CHECK_EQ(mst2.end(), mst2.find(3));
  }
  {
    Multiset<int> mst1, mst2;
    mst1.insert(1);
    mst1.insert(2);
    mst2 = std::move(mst1);
    CHECK_EQ(2, mst2.size());
    CHECK_EQ(true, mst1.empty());
  }
}

// 测试迭代器的双向移动
static void multiset_iterator_impl() {
  Multiset<int> mst;
  mst.insert(1);
  mst.insert(2);
  mst.insert(3);

  auto it = mst.end();
  --it;
  CHECK_EQ(3, *it);
  --it;
  CHECK_EQ(2, *it);
  --it;
  CHECK_EQ(1, *it);
  CHECK_EQ(it, mst.begin());

  // 验证 const 迭代器
  const Multiset<int>& cmst = mst;
  auto cit = cmst.end();
  --cit;
  CHECK_EQ(3, *cit);
}

// 测试非基本类型 (std::string)
static void multiset_string_impl() {
  Multiset<std::string> mst;
  mst.insert("apple");
  mst.insert("banana");
  mst.insert("apple"); // 重复插入

  CHECK_EQ(3, mst.size());
  CHECK_EQ("apple", *mst.begin());
  
  // 按值删除，应该删除两个 "apple"
  CHECK_EQ(2, mst.erase("apple"));
  CHECK_EQ(1, mst.size());
  CHECK_EQ("banana", *mst.begin());
}

// 测试大量重复元素及 equalRange
static void multiset_duplicates_impl() {
  Multiset<int> mst;
  const int count = 10;
  for (int i = 0; i < count; ++i) {
    mst.insert(100);
  }
  mst.insert(50);
  mst.insert(150);

  CHECK_EQ(count + 2, mst.size());

  // 测试 equalRange
  auto range = mst.equalRange(100);
  int range_count = 0;
  for (auto it = range.first; it != range.second; ++it) {
    CHECK_EQ(100, *it);
    range_count++;
  }
  CHECK_EQ(count, range_count);

  // 验证边界
  CHECK_EQ(50, *(--range.first));
  CHECK_EQ(150, *(range.second));

  // 测试删除所有重复项
  auto erased_count = mst.erase(100);
  CHECK_EQ(count, erased_count);
  CHECK_EQ(2, mst.size());
  CHECK_EQ(mst.end(), mst.find(100));
}

// 测试边界删除和返回值
static void multiset_erase_edge_impl() {
  Multiset<int> mst;
  mst.insert(10);
  mst.insert(20);
  mst.insert(30);

  // 删除中间元素，检查返回值
  auto it = mst.find(20);
  auto next = mst.erase(it);
  CHECK_EQ(30, *next);
  CHECK_EQ(2, mst.size());

  // 删除最后一个元素，返回值应为 end()
  it = mst.find(30);
  next = mst.erase(it);
  CHECK_EQ(mst.end(), next);
  CHECK_EQ(1, mst.size());

  // 删除第一个元素 (也就是仅剩的 10)
  it = mst.begin();
  CHECK_EQ(10, *it);
  next = mst.erase(it);
  CHECK_EQ(mst.end(), next);
  CHECK_EQ(true, mst.empty());
}
namespace {
struct MoveOnly {
  int val;
  explicit MoveOnly(int v) : val(v) {}
  MoveOnly(MoveOnly&&) = default;
  MoveOnly& operator=(MoveOnly&&) = default;
  MoveOnly(const MoveOnly&) = delete;
  MoveOnly& operator=(const MoveOnly&) = delete;

  // 比较函数
  bool operator<(const MoveOnly& other) const { return val < other.val; }
};
struct AbsCompare {
  bool operator()(int a, int b) const {
    return std::abs(a) < std::abs(b);
  }
};
}

static void multiset_move_only_impl() {
  Multiset<MoveOnly> mst;
  mst.insert(MoveOnly(10));
  mst.insert(MoveOnly(5));
  mst.insert(MoveOnly(10));

  CHECK_EQ(3, mst.size());
  
  auto it = mst.begin();
  CHECK_EQ(5, it->val);
  ++it;
  CHECK_EQ(10, it->val);
  
  mst.erase(it);
  CHECK_EQ(2, mst.size());
  CHECK_EQ(5, mst.begin()->val);
}

static void multiset_custom_compare_impl() {
  Multiset<int, AbsCompare> mst;
  mst.insert(-10);
  mst.insert(5);
  mst.insert(10); // abs(10) == abs(-10)，语义上这是重复元素

  // 顺序应该是 5, -10, 10 (或者 5, 10, -10，取决于插入顺序和红黑树调整)
  // 但绝对值必须是有序的：5 < 10
  CHECK_EQ(3, mst.size());
  CHECK_EQ(5, *mst.begin());
  
  // find 应该能找到 -10 和 10
  CHECK_EQ(true, mst.find(10) != mst.end());
  CHECK_EQ(true, mst.find(-10) != mst.end());
  
  // 验证 equalRange 会同时囊括 10 和 -10
  auto range = mst.equalRange(10); // 找绝对值为 10 的
  int count = 0;
  for(auto it = range.first; it != range.second; ++it) {
      CHECK_EQ(10, std::abs(*it));
      count++;
  }
  CHECK_EQ(2, count);
}

static void multiset_self_assignment_impl() {
  Multiset<int> mst;
  mst.insert(1);
  mst.insert(2);

  mst = mst;
  
  CHECK_EQ(2, mst.size());
  CHECK_EQ(1, *mst.begin());
  
  mst = std::move(mst);
  CHECK_EQ(2, mst.size()); 
}

static void multiset_const_impl() {
  Multiset<int> mst;
  mst.insert(10);
  mst.insert(20);
  mst.insert(20);
  mst.insert(30);

  // 创建一个 const 引用
  // 后续所有操作都会强制调用 const 版本的成员函数
  const Multiset<int>& cmst = mst;

  // 1. 测试基础属性 (调用 size() const, empty() const)
  CHECK_EQ(4, cmst.size());
  CHECK_EQ(false, cmst.empty());

  // 2. 测试迭代器 (调用 begin() const)
  // 如果实现正确，it 的类型应该是 Multiset<int>::ConstIterator
  auto it = cmst.begin();
  CHECK_EQ(10, *it);
  
  // 验证迭代器移动
  ++it;
  CHECK_EQ(20, *it);

  // 3. 测试查找
  auto fit = cmst.find(30);
  CHECK_EQ(false, fit == cmst.end());
  CHECK_EQ(30, *fit);

  auto fit_fail = cmst.find(999);
  CHECK_EQ(true, fit_fail == cmst.end());

  // 4. 测试范围查找
  auto range = cmst.equalRange(20);
  // range.first 应该是第一个 20
  CHECK_EQ(20, *range.first);
  // range.second 应该是 30
  CHECK_EQ(30, *range.second);
  
  // 验证距离为 2
  int count = 0;
  for (auto i = range.first; i != range.second; ++i) {
      count++;
  }
  CHECK_EQ(2, count);
}

static void multiset_basic_impl() {
  // insert and order
  Multiset<int> a;
  a.insert(1);
  a.insert(2);
  a.insert(2);
  a.insert(3);
  CHECK_EQ(4, a.size());
  std::vector<int> vals;
  for (auto it = a.begin(); it != a.end(); ++it) vals.push_back(*it);
  CHECK_EQ(1, vals[0]);
  CHECK_EQ(2, vals[1]);
  CHECK_EQ(2, vals[2]);
  CHECK_EQ(3, vals[3]);

  // erase by iterator
  auto it2 = a.find(2);
  CHECK_EQ(2, *it2);
  auto next_it = a.erase(it2);
  CHECK_EQ(3, *next_it);
  CHECK_EQ(3, a.size());

  // erase by value (remove remaining '2')
  a.insert(2);
  auto removed = a.erase(2);
  CHECK_EQ(2, removed);
  CHECK_EQ(2, a.size());

  // bounds and equalRange
  a.insert(2);
  a.insert(2);
  auto lb = a.lowerBound(2);
  auto ub = a.upperBound(2);
  CHECK_EQ(2, *lb);
  // distance between lb and ub should be number of 2s
  int count2 = 0;
  for (auto it = lb; it != ub; ++it) ++count2;
  CHECK_EQ(2, count2);

  auto eq = a.equalRange(2);
  CHECK_EQ(*eq.first, 2);

  // find non-existing
  auto f = a.find(999);
  CHECK_EQ(a.end(), f);

  // clear
  a.clear();
  CHECK_EQ(true, a.empty());
  CHECK_EQ(a.begin(), a.end());

  // swap
  Multiset<int> b;
  b.insert(10);
  b.insert(20);
  a.insert(1);
  a.insert(2);
  a.swap(b);
  CHECK_EQ(2, b.size());
  CHECK_EQ(2, a.size());
  // after swap, a should have elements 10,20
  vals.clear();
  for (auto it = a.begin(); it != a.end(); ++it) vals.push_back(*it);
  CHECK_EQ(10, vals[0]);
  CHECK_EQ(20, vals[1]);
}

static void multiset_fuzzy_impl() {
  RandomGenerator gen;
  const int query_times = 1000000;
  const int num_range = 100;
  Multiset<int> st;
  multiset<int> ref;
  for (int t = 0; t < query_times; t++) {
    int opt = gen.uniform_int(0, 5);
    int rndint = gen.uniform_int(0, num_range);
    switch (opt) {
      case 0: {
        CHECK_EQ(ref.empty(), st.empty());
        break;
      }
      case 1: {
        CHECK_EQ(ref.size(), st.size());
        break;
      }
      case 2: {
        auto my_ans = st.insert(rndint);
        auto ref_ans = ref.insert(rndint);
        CHECK_EQ(*ref_ans, *my_ans);
        break;
      }
      case 3: {
        auto my_ans = st.erase(rndint);
        auto ref_ans = ref.erase(rndint);
        CHECK_EQ(ref_ans, my_ans);
        break;
      }
      case 4: {
        auto my_ans = (st.find(rndint) == st.end());
        auto ref_ans = (ref.find(rndint) == ref.end());
        CHECK_EQ(ref_ans, my_ans);
        break;
      }
      case 5: {
        auto my_ans = st.equalRange(rndint);
        auto ref_ans = ref.equal_range(rndint);
        if (ref_ans.first == ref.end()) {
          CHECK_EQ(true, (my_ans.first == st.end()));
        } else {
          CHECK_EQ((*ref_ans.first), (*my_ans.first));
        }
        if (ref_ans.second == ref.end()) {
          CHECK_EQ(true, (my_ans.second == st.end()));
        } else {
          CHECK_EQ((*ref_ans.second), (*my_ans.second));
        }
        break;
      }
      default:
        break;
    }
  }
}

// register tests
MAKE_TEST(Multiset, Construct) { multiset_construct_impl(); }
MAKE_TEST(Multiset, Iterator) { multiset_iterator_impl(); }
MAKE_TEST(Multiset, StringType) { multiset_string_impl(); }
MAKE_TEST(Multiset, Duplicates) { multiset_duplicates_impl(); }
MAKE_TEST(Multiset, EraseEdge) { multiset_erase_edge_impl(); }
MAKE_TEST(Multiset, MoveOnly) { multiset_move_only_impl(); }
MAKE_TEST(Multiset, CustomCompare) { multiset_custom_compare_impl(); }
MAKE_TEST(Multiset, SelfAssign) { multiset_self_assignment_impl(); }
MAKE_TEST(Multiset, ConstSupport) { multiset_const_impl(); }
MAKE_TEST(Multiset, Basics) { multiset_basic_impl(); }
MAKE_TEST(Multiset, Fuzzy) { multiset_fuzzy_impl(); }