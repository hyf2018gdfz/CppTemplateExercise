#ifndef FENWICKTREE_HPP
#define FENWICKTREE_HPP

#include "common.h"

#include <vector>

namespace mystd {

// op: T × T -> T，满足结合律，e 是单位元
// opInv: 两段前缀和做差
template <typename T, T (*e)(), T (*op)(T, T), T (*opInv)(T, T)>
class FenwickTree {
private:
    int n;
    std::vector<T> tree;

    static constexpr int lowbit(int x) { return x & -x; }

public:
    explicit FenwickTree(int n) : FenwickTree(std::vector<T>(n, e())) {}
    explicit FenwickTree(const std::vector<T> &arr) : n(int(arr.size())) {
        tree.resize(n + 1, e());
        for (int i = 1; i <= n; i++) {
            tree[i] = op(tree[i], arr[i - 1]);
            int j = i + lowbit(i);
            if (j <= n) tree[j] = op(tree[j], tree[i]);
        }
    }

    void point_add(int index, T value) {
        for (; index <= n; index += lowbit(index)) {
            tree[index] = op(tree[index], value);
        }
    }

    T query(int index) const {
        T result = e();
        for (; index > 0; index -= lowbit(index)) {
            result = op(result, tree[index]);
        }
        return result;
    }

    T interval_query(int L, int R) const {
        if (L < 1 || R > n || L > R)
            throw std::out_of_range("Invalid interval");
        return opInv(query(R), query(L - 1));
    }
};
} // namespace mystd

#endif // FENWICKTREE_HPP