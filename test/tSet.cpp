#include <set>

#include "Set.hpp"
#include "test.h"
using mystd::set::Set;
using std::set;

static void set_construct_impl() {
  // 默认构造
  {
    Set<int> st;
    CHECK_EQ(0, st.size());
    CHECK_EQ(true, st.empty());
    CHECK_EQ(st.begin(), st.end());
  }

  // 自定义比较器构造
  {
    Set<int, mystd::compare::Greater<int>> st;
    st.insert(10);
    st.insert(20);
    st.insert(5);
    // Greater: 20 -> 10 -> 5
    auto it = st.begin();
    CHECK_EQ(20, *it);
    CHECK_EQ(10, *(++it));
  }

  // 拷贝构造
  {
    Set<int> st1;
    st1.insert(1);
    st1.insert(2);
    Set<int> st2(st1);
    
    CHECK_EQ(2, st2.size());
    st1.insert(3);
    CHECK_EQ(3, st1.size());
    CHECK_EQ(2, st2.size());
  }

  // 移动构造
  {
    Set<int> st1;
    st1.insert(1);
    st1.insert(2);
    Set<int> st2(std::move(st1));
    
    CHECK_EQ(2, st2.size());
    CHECK_EQ(true, st1.empty());
  }

  // 拷贝赋值与自赋值
  {
    Set<int> st1, st2;
    st1.insert(10);
    st2 = st1;
    CHECK_EQ(1, st2.size());
    
    st1.insert(20);
    st2 = st2;
    CHECK_EQ(1, st2.size());
    CHECK_EQ(2, st1.size());
  }

  // 移动赋值
  {
    Set<int> st1, st2;
    st1.insert(10);
    st2.insert(5);
    st2 = std::move(st1);
    
    CHECK_EQ(1, st2.size());
    CHECK_EQ(10, *st2.begin());
    CHECK_EQ(true, st1.empty());
  }
}

static void set_basic_impl() {
  Set<int> st;

  // 测试 Insert 和 唯一性
  // 第一次插入
  auto ret1 = st.insert(10);
  CHECK_EQ(true, ret1.second);
  CHECK_EQ(10, *ret1.first);
  CHECK_EQ(1, st.size());

  // 插入重复元素
  auto ret2 = st.insert(10);
  CHECK_EQ(false, ret2.second);
  CHECK_EQ(10, *ret2.first);
  CHECK_EQ(1, st.size());

  // 插入新元素
  st.insert(20);
  st.insert(30);
  CHECK_EQ(3, st.size());

  // 验证有序性
  std::vector<int> vec;
  for(auto x : st) vec.push_back(x);
  CHECK_EQ(10, vec[0]);
  CHECK_EQ(20, vec[1]);
  CHECK_EQ(30, vec[2]);

  // 测试 Erase (迭代器)
  auto it = st.find(20);
  auto next = st.erase(it);
  CHECK_EQ(30, *next); 
  CHECK_EQ(2, st.size());

  // 测试 Erase (值)
  size_t cnt = st.erase(10);
  CHECK_EQ(1, cnt);
  CHECK_EQ(1, st.size());

  // 测试删除不存在的值
  cnt = st.erase(999);
  CHECK_EQ(0, cnt);
  CHECK_EQ(1, st.size());

  // 测试 Clear
  st.clear();
  CHECK_EQ(true, st.empty());

  // 测试 Swap
  Set<int> a, b;
  a.insert(1);
  b.insert(2); b.insert(3);
  a.swap(b);
  CHECK_EQ(2, a.size());
  CHECK_EQ(1, b.size());
  CHECK_EQ(2, *a.begin());
}

static void set_iterator_impl() {
  Set<int> st;
  st.insert(10);
  st.insert(20);
  st.insert(30);

  // 1. operator++
  auto it = st.begin();
  CHECK_EQ(10, *it);
  CHECK_EQ(20, *(++it));
  CHECK_EQ(30, *(++it));
  CHECK_EQ(st.end(), ++it);

  // 2. operator-- (from end)
  auto end_it = st.end();
  CHECK_EQ(30, *(--end_it));
  CHECK_EQ(20, *(--end_it));
  CHECK_EQ(10, *(--end_it));
  CHECK_EQ(st.begin(), end_it);
  
  // 3. const_iterator
  const Set<int>& cst = st;
  auto cit = cst.end();
  CHECK_EQ(30, *(--cit));
}

namespace {
struct MoveOnly {
  int val;
  explicit MoveOnly(int v) : val(v) {}
  MoveOnly(MoveOnly&&) = default;
  MoveOnly& operator=(MoveOnly&&) = default;
  MoveOnly(const MoveOnly&) = delete;
  MoveOnly& operator=(const MoveOnly&) = delete;

  bool operator<(const MoveOnly& other) const { return val < other.val; }
};
}

static void set_move_only_impl() {
  Set<MoveOnly> st;

  // 1. insert(T&&)
  auto ret = st.insert(MoveOnly(10));
  CHECK_EQ(true, ret.second);
  CHECK_EQ(10, ret.first->val);

  // 2. Duplicate insert
  ret = st.insert(MoveOnly(10));
  CHECK_EQ(false, ret.second);
  CHECK_EQ(10, ret.first->val);
  CHECK_EQ(1, st.size());

  st.insert(MoveOnly(5));
  st.insert(MoveOnly(15)); // {5, 10, 15}

  // 3. erase(iterator)
  auto it = st.find(MoveOnly(10)); // uses temporary for lookup
  auto next = st.erase(it);
  CHECK_EQ(15, next->val);
  CHECK_EQ(2, st.size());

  // 4. erase(const Key&) - by value
  size_t cnt = st.erase(MoveOnly(5));
  CHECK_EQ(1, cnt);
  CHECK_EQ(1, st.size());
  CHECK_EQ(15, st.begin()->val);

  // 5. Swap
  Set<MoveOnly> st2;
  st2.insert(MoveOnly(99));
  st.swap(st2);
  CHECK_EQ(99, st.begin()->val); // st got 99
  CHECK_EQ(15, st2.begin()->val); // st2 got 15
}

static void set_fuzzy_impl() {
  RandomGenerator gen;
  const int query_times = 1000000;
  const int num_range = 1000;
  Set<int> st;
  set<int> ref;
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
        CHECK_EQ(*ref_ans.first, *my_ans.first);
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
MAKE_TEST(Set, Construct) { set_construct_impl(); }
MAKE_TEST(Set, Basic) { set_basic_impl(); }
MAKE_TEST(Set, Iterator) { set_iterator_impl(); }
MAKE_TEST(Set, MoveOnly) { set_move_only_impl(); }
MAKE_TEST(Set, Fuzzy) { set_fuzzy_impl(); }