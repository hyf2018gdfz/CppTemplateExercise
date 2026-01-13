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

// register tests
MAKE_TEST(Multiset, Construct) { multiset_construct_impl(); }
MAKE_TEST(Multiset, Fuzzy) { multiset_fuzzy_impl(); }
MAKE_TEST(Multiset, Basics) { multiset_basic_impl(); }