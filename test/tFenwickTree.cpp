#include <stdexcept>
#include <vector>

#include "FenwickTree.hpp"
#include "common.h"
#include "test.h"
#include "testcase.h"

namespace TestFenwickTree {
const int mod = 1e9 + 7;
auto e = []() { return 0; };
auto add = [](int x, int y) { return (x + y) % mod; };
auto sub = [](int x, int y) { return ((x - y) % mod + mod) % mod; };
};  // namespace TestFenwickTree
using namespace TestFenwickTree;

using namespace mystd::fenwick_tree;

void test_FenwickTree() {
  RandomGenerator gen;
  std::vector<int> arr;
  const int arr_len = 10000;
  const int query_times = 100000;
  const int num_range = 100000;
  for (int i = 0; i < arr_len; i++) {
    arr.push_back(gen.uniform_int(0, num_range));
  }
  FenwickTree<int, e, add, sub> tree(arr);
  std::vector<int> ref = arr;
  EXPECT_THROW(tree.query(100, 1), std::out_of_range);
  EXPECT_THROW(tree.query(-2), std::out_of_range);
  EXPECT_THROW(tree.apply(-1, 100), std::out_of_range);
  for (int q = 0; q < query_times; q++) {
    int opt = gen.uniform_int(0, 2);
    int l = gen.uniform_int(0, arr_len - 1);
    int r = gen.uniform_int(0, arr_len - 1);
    if (l > r) mystd::swap(l, r);
    switch (opt) {
      case 0: {
        int val = gen.uniform_int(0, num_range);
        tree.apply(l, val);
        ref[l] = add(ref[l], val);
        break;
      }
      case 1: {
        int ans1 = tree.query(l);
        int ans2 = e();
        for (int i = 0; i <= l; i++) {
          ans2 = add(ans2, ref[i]);
        }
        CHECK_EQ(ans2, ans1);
        break;
      }
      case 2: {
        int ans1 = tree.query(l, r);
        int ans2 = e();
        for (int i = l; i <= r; i++) {
          ans2 = add(ans2, ref[i]);
        }
        CHECK_EQ(ans2, ans1);
        break;
      }
      default:
        break;
    }
  }
}