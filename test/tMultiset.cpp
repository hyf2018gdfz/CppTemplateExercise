#include <set>

#include "Compare.hpp"
#include "Set.hpp"
#include "test.h"
#include "testcase.h"
using mystd::set::Multiset;
using std::multiset;

static void test_construct() {
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

void test_Multiset() {
  test_construct();
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