#include "SegmentTree.hpp"

namespace TestSegmentTree {
auto e = []() { return 0; };
auto add = [](int x, int y) { return x + y; };
auto mapping = [](int x, int y) { return x * y; };
} // namespace TestSegmentTree
using namespace TestSegmentTree;

void test_SegmentTree() {
    SegmentTree<int, e, add, mapping> segTree({1, 2, 3, 4, 5});
    CHECK_EQ(15, segTree.interval_query(1, 5));
    segTree.interval_add(2, 4, 3);
    CHECK_EQ(24, segTree.interval_query(1, 5));
    CHECK_EQ(5, segTree.point_query(2));
    segTree.point_add(3, 2);
    CHECK_EQ(8, segTree.point_query(3));
    CHECK_EQ(26, segTree.interval_query(1, 5));
}

void test_SegmentTree_big() {
    int n = 100000;
    vector<int> data(n, 1);
    SegmentTree<int, e, add, mapping> segTree(data);
    CHECK_EQ(n, segTree.interval_query(1, n));
    segTree.interval_add(1, n / 2, 2);
    // total = n + (n/2) * 2
    CHECK_EQ(n * 2, segTree.interval_query(1, n));
    segTree.interval_add(n / 4, 3 * n / 4, 3);
    CHECK_EQ(6, segTree.point_query(n / 4));
    CHECK_EQ(4, segTree.point_query(3 * n / 4));
    // total = n + (n/2) * 2 + (n/2+1) * 3
    CHECK_EQ(n * 2 + (n / 2 + 1) * 3, segTree.interval_query(1, n));
}