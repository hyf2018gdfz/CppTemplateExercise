#include "FenwickTree.hpp"

namespace TestFenwickTree {
auto e = []() { return 0; };
auto add = [](int x, int y) { return x + y; };
auto sub = [](int x, int y) { return x - y; };
}; // namespace TestFenwickTree
using namespace TestFenwickTree;

void test_FenwickTree() {
    mystd::FenwickTree<int, e, add, sub> tree({1, 2, 3, 4, 5});
    CHECK_EQ(15, tree.interval_query(1, 5));
    tree.point_add(3, 2);
    CHECK_EQ(16, tree.interval_query(2, 5));
    tree.point_add(1, 5);
    CHECK_EQ(17, tree.interval_query(1, 4));
}