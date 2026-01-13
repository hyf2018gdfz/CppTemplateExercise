#include <cstdlib>
#include <ctime>
#include <sstream>
#include <vector>

#include "LinkList.hpp"
#include "test.h"

using namespace mystd::linklist;

static std::string vec_to_string(const std::vector<int> &v) {
  std::ostringstream oss;
  oss << "[";
  for (size_t i = 0; i < v.size(); ++i) {
    if (i) oss << ", ";
    oss << v[i];
  }
  oss << "]";
  return oss.str();
}

static void full_compare(const LinkList<int> &lst,
                         const std::vector<int> &ref) {
  CHECK_EQ(ref.size(), lst.size());
  for (size_t i = 0; i < ref.size(); ++i) {
    CHECK_EQ(ref[i], lst[i]);
  }
  std::ostringstream oss;
  oss << lst;
  CHECK_EQ(vec_to_string(ref), oss.str());
}

void test_LinkList() {
  RandomGenerator gen;
  LinkList<int> lst;
  std::vector<int> ref;

  const int OPS = 200000;
  const int MAX_VAL = 1000000;

  for (int it = 0; it < OPS; ++it) {
    int op = gen.uniform_int(0, 7);
    if (op == 0) {  // push_back
      int x = gen.uniform_int(0, MAX_VAL);
      lst.pushBack(x);
      ref.push_back(x);
    } else if (op == 1) {  // push_front
      int x = gen.uniform_int(0, MAX_VAL);
      lst.pushFront(x);
      ref.insert(ref.begin(), x);
    } else if (op == 2) {  // pop_back
      if (ref.empty()) {
        EXPECT_THROW(lst.popBack(), std::out_of_range);
      } else {
        lst.popBack();
        ref.pop_back();
      }
    } else if (op == 3) {  // pop_front
      if (ref.empty()) {
        EXPECT_THROW(lst.popFront(), std::out_of_range);
      } else {
        lst.popFront();
        ref.erase(ref.begin());
      }
    } else if (op == 4) {  // insert
      size_t pos = gen.uniform_int(0ul, ref.size());
      int x = gen.uniform_int(0, MAX_VAL);
      lst.insert(pos, x);
      ref.insert(ref.begin() + pos, x);
    } else if (op == 5) {  // erase
      if (ref.empty()) {
        EXPECT_THROW(lst.erase(0), std::out_of_range);
      } else {
        size_t pos = gen.uniform_int(0ul, ref.size() - 1);
        lst.erase(pos);
        ref.erase(ref.begin() + pos);
      }
    } else if (op == 6) {  // find / operator[] / out_of_range
      if (ref.empty()) {
        EXPECT_THROW(lst[0], std::out_of_range);
        CHECK_EQ(-1, lst.find(12345678));
      } else {
        size_t pos = gen.uniform_int(0ul, ref.size() - 1);
        CHECK_EQ(ref[pos], lst[pos]);
        CHECK_EQ(static_cast<ptrdiff_t>(pos), lst.find(ref[pos]));
        // search for non-existing value
        int v = 12345678;
        CHECK_EQ(-1, lst.find(v));
      }
    } else {  // clear
      lst.clear();
      ref.clear();
    }

    if ((it & 0xFF) == 0) {
      full_compare(lst, ref);
    }
  }
  full_compare(lst, ref);
}

// register tests
MAKE_TEST(LinkList, Default) { test_LinkList(); }