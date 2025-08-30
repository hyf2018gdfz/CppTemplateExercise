#ifndef SEGMENTTREE_HPP
#define SEGMENTTREE_HPP

#include "common.h"

// op: T x T -> T，满足结合律，e 是单位元
// mapping: 二元组 (lazy[o], length_of_the_interval) 对 tree[o] 的贡献
template <typename T, T (*e)(), T (*op)(T, T), T (*mapping)(T, int)>
class SegmentTree {
private:
    int n;
    vector<T> tree, lazy;
    void push_up(int o) { tree[o] = op(tree[o * 2], tree[o * 2 + 1]); }
    void push_down(int o, int l, int r) {
        if (lazy[o] == e()) return;
        int mid = (l + r) / 2;
        tree[o * 2] = op(tree[o * 2], mapping(lazy[o], mid - l + 1));
        tree[o * 2 + 1] = op(tree[o * 2 + 1], mapping(lazy[o], r - mid));
        lazy[o * 2] = op(lazy[o * 2], lazy[o]);
        lazy[o * 2 + 1] = op(lazy[o * 2 + 1], lazy[o]);
        lazy[o] = e();
    }
    void build(int l, int r, int o, const vector<T> &arr) {
        if (l == r) {
            tree[o] = arr[l - 1];
            return;
        }
        int mid = (l + r) / 2;
        build(l, mid, o * 2, arr);
        build(mid + 1, r, o * 2 + 1, arr);
        push_up(o);
    }
    void _point_add(int index, T num, int l, int r, int o) {
        if (l == r) {
            tree[o] = op(tree[o], num);
            return;
        }
        int mid = (l + r) / 2;
        if (index <= mid)
            _point_add(index, num, l, mid, o * 2);
        else
            _point_add(index, num, mid + 1, r, o * 2 + 1);
        push_up(o);
    }
    T _point_query(int index, int l, int r, int o) {
        if (l == r) { return tree[o]; }
        push_down(o, l, r);
        int mid = (l + r) / 2;
        if (index <= mid)
            return _point_query(index, l, mid, o * 2);
        else
            return _point_query(index, mid + 1, r, o * 2 + 1);
    }
    void _interval_add(int L, int R, T num, int l, int r, int o) {
        if (L <= l && r <= R) {
            tree[o] = op(tree[o], mapping(num, r - l + 1));
            lazy[o] = op(lazy[o], num);
            return;
        }
        push_down(o, l, r);
        int mid = (l + r) / 2;
        if (L <= mid) _interval_add(L, R, num, l, mid, o * 2);
        if (R > mid) _interval_add(L, R, num, mid + 1, r, o * 2 + 1);
        push_up(o);
    }
    T _interval_query(int L, int R, int l, int r, int o) {
        if (L <= l && r <= R) { return tree[o]; }
        push_down(o, l, r);
        int mid = (l + r) / 2;
        T ans = e();
        if (L <= mid) ans = op(ans, _interval_query(L, R, l, mid, o * 2));
        if (R > mid)
            ans = op(ans, _interval_query(L, R, mid + 1, r, o * 2 + 1));
        return ans;
    }

public:
    explicit SegmentTree(int n) : SegmentTree(vector<T>(n, e())) {}
    explicit SegmentTree(const vector<T> &arr) : n(int(arr.size())) {
        tree.resize(4 * n + 1);
        lazy.resize(4 * n + 1, e());
        build(1, n, 1, arr);
    }
    void point_add(int index, T num) {
        if (index < 1 || index > n) throw out_of_range("Index out of bounds");
        _point_add(index, num, 1, n, 1);
    }
    T point_query(int index) {
        if (index < 1 || index > n) throw out_of_range("Index out of bounds");
        return _point_query(index, 1, n, 1);
    }
    void interval_add(int L, int R, T num) {
        if (L < 1 || R > n || L > R) throw out_of_range("Invalid interval");
        _interval_add(L, R, num, 1, n, 1);
    }
    T interval_query(int L, int R) {
        if (L < 1 || R > n || L > R) throw out_of_range("Invalid interval");
        return _interval_query(L, R, 1, n, 1);
    }
};

#endif // SEGMENTTREE_HPP