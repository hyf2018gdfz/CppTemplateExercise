#include <vector>

#include "SegmentTree.hpp"
#include "test.h"
#include "testcase.h"

namespace TestSegmentTree {
const int mod = 1e9 + 7;
// 区间修改 x <- x * a + b、区间求和，单点赋值、修改、查询，答案对 mod 取模
// 参考：https://atcoder.jp/contests/practice2/editorial/100
struct node {
  long long a;
  int size;
};
struct func {
  long long a, b;
};
node op(node l, node r) { return node{(l.a + r.a) % mod, l.size + r.size}; }
node e() { return node{0, 0}; }
node mapping(func fn, node nd) {
  return node{(nd.a * fn.a % mod + nd.size * fn.b % mod) % mod, nd.size};
}
func composition(func new_fn, func old_fn) {
  return func{new_fn.a * old_fn.a % mod,
              (new_fn.a * old_fn.b % mod + new_fn.b) % mod};
}
func id() { return func{1, 0}; }
}  // namespace TestSegmentTree
using namespace TestSegmentTree;

using namespace mystd::segment_tree;

void test_SegmentTree() {
  RandomGenerator gen;
  std::vector<node> arr;
  const int arr_len = 10000;
  const int query_times = 100000;
  const int num_range = 100000;
  for (int i = 0; i < arr_len; i++) {
    arr.push_back(node{gen.uniform_int(0, num_range), 1});
  }
  SegmentTree<node, op, e, func, mapping, composition, id> seg(arr);
  std::vector<node> ref = arr;  // 用暴力算法作为ref对比
  for (int i = 0; i < query_times; i++) {
    int opt = gen.uniform_int(0, 4);
    int l = gen.uniform_int(0, arr_len - 1);
    int r = gen.uniform_int(0, arr_len - 1);
    if (l > r) std::swap(l, r);
    switch (opt) {
      case 0: {
        func f{gen.uniform_int(0, num_range), gen.uniform_int(0, num_range)};
        seg.apply(l, r, f);
        for (int j = l; j <= r; j++) {
          ref[j].a = (ref[j].a * f.a % mod + f.b) % mod;
        }
        break;
      }
      case 1: {
        func f{gen.uniform_int(0, num_range), gen.uniform_int(0, num_range)};
        seg.apply(l, f);
        ref[l].a = (ref[l].a * f.a % mod + f.b) % mod;
        break;
      }
      case 2: {
        node ans1 = seg.query(l, r);
        node ans2 = e();
        for (int j = l; j <= r; j++) {
          ans2 = op(ans2, ref[j]);
        }
        CHECK_EQ(ans2.a, ans1.a);
      }
      case 3: {
        node ans1 = seg.query(l);
        node ans2 = ref[l];
        CHECK_EQ(ans2.a, ans1.a);
      }
      case 4: {
        node val{gen.uniform_int(0, num_range), 1};
        seg.assign(l, val);
        ref[l] = val;
        break;
      }
      default:
        break;
    }
  }
}