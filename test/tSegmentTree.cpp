#include "SegmentTree.hpp"
#include "test.h"

#include <vector>

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
node op(node l, node r) {
    return node{(l.a + r.a) % mod, l.size + r.size};
}
node e() {
    return node{0, 0};
}
node mapping(func l, node r) {
    return node{(r.a * l.a % mod + r.size * l.b % mod) % mod, r.size};
}
func composition(func l, func r) {
    return func{r.a * l.a % mod, (r.b * l.a % mod + l.b) % mod};
}
func id() {
    return func{1, 0};
}
} // namespace TestSegmentTree
using namespace TestSegmentTree;

void test_SegmentTree() {
    srand(time(0));
    std::vector<node> arr;
    static int arr_len = 10000;
    static int query_times = 100000;
    static int num_range = 100000;
    for (int i = 0; i < arr_len; i++) {
        arr.push_back(node{rand() % num_range, 1});
    }
    mystd::SegmentTree<node, op, e, func, mapping, composition, id> seg(arr);
    std::vector<node> brute = arr;
    for (int i = 0; i < query_times; i++) {
        int opt = rand() % 5;
        int l = rand() % arr_len;
        int r = rand() % arr_len;
        if (l > r) std::swap(l, r);
        switch (opt) {
        case 0: {
            func f{rand() % num_range, rand() % num_range};
            seg.apply(l, r, f);
            for (int j = l; j <= r; j++) {
                brute[j].a = (brute[j].a * f.a % mod + f.b) % mod;
            }
            break;
        }
        case 1: {
            func f{rand() % num_range, rand() % num_range};
            seg.apply(l, f);
            brute[l].a = (brute[l].a * f.a % mod + f.b) % mod;
            break;
        }
        case 2: {
            node ans1 = seg.query(l, r);
            node ans2 = e();
            for (int j = l; j <= r; j++) { ans2 = op(ans2, brute[j]); }
            CHECK_EQ(ans2.a, ans1.a);
        }
        case 3: {
            node ans1 = seg.query(l);
            node ans2 = brute[l];
            CHECK_EQ(ans2.a, ans1.a);
        }
        case 4: {
            node val{rand() % num_range, 1};
            seg.assign(l, val);
            brute[l] = val;
            break;
        }
        default: break;
        }
    }
}