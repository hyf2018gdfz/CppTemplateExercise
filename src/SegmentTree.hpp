#ifndef SEGMENTTREE_HPP
#define SEGMENTTREE_HPP

#include "BitOperation.hpp"

#include <vector>

namespace mystd {

template <typename T,
          T (*op)(T, T),
          T (*e)(),
          typename F,
          T (*mapping)(F, T),
          F (*composition)(F, F),
          F (*id)()>
class SegmentTree {
private:
    int n, size, log;
    // 为了方便在算法竞赛环境中使用，使用 std::vector 而不是 mystd::Vector
    std::vector<T> tree;
    std::vector<F> lazy;
    void push_up(int o) { tree[o] = op(tree[o * 2], tree[o * 2 + 1]); }
    void apply_node(int o, F f) {
        tree[o] = mapping(f, tree[o]);
        if (o < size) lazy[o] = composition(f, lazy[o]);
    }
    void push_down(int o) {
        apply_node(o * 2, lazy[o]);
        apply_node(o * 2 + 1, lazy[o]);
        lazy[o] = id();
    }

public:
    explicit SegmentTree(int n) : SegmentTree(std::vector<T>(n, e())) {}
    // 数组下标从 0 开始
    explicit SegmentTree(const std::vector<T> &arr) : n((int)arr.size()) {
        size = bitop::bit_ceil(n);
        log = bitop::countr_zero(size);
        tree.resize(size * 2, e());
        lazy.resize(size, id());
        for (int i = 0; i < n; i++) tree[size + i] = arr[i];
        for (int i = size - 1; i > 0; i--) push_up(i);
    }
    void assign(int p, T val) {
        if (p < 0 || p >= n) throw std::out_of_range("Index out of range");
        p += size;
        for (int i = log; i >= 1; i--) push_down(p >> i);
        tree[p] = val;
        for (int i = 1; i <= log; i++) push_up(p >> i);
    }
    void apply(int p, F f) {
        if (p < 0 || p >= n) throw std::out_of_range("Index out of range");
        p += size;
        for (int i = log; i >= 1; i--) push_down(p >> i);
        tree[p] = mapping(f, tree[p]);
        for (int i = 1; i <= log; i++) push_up(p >> i);
    }
    // 区间为闭区间 [L, R]，但内部实现的时候使用半开区间 [L, R+1)，下同
    void apply(int L, int R, F f) {
        if (L < 0 || R >= n || L > R)
            throw std::out_of_range("Invalid interval");
        L += size;
        R += size + 1;
        for (int i = log; i >= 1; i--) {
            if (((L >> i) << i) != L) push_down(L >> i);
            if (((R >> i) << i) != R) push_down((R - 1) >> i);
        }
        int L0 = L, R0 = R;
        while (L < R) {
            if (L & 1) apply_node(L++, f);
            if (R & 1) apply_node(--R, f);
            L >>= 1;
            R >>= 1;
        }
        for (int i = 1; i <= log; i++) {
            if (((L0 >> i) << i) != L0) push_up(L0 >> i);
            if (((R0 >> i) << i) != R0) push_up((R0 - 1) >> i);
        }
    }
    T query(int p) {
        if (p < 0 || p >= n) throw std::out_of_range("Index out of range");
        p += size;
        for (int i = log; i >= 1; i--) push_down(p >> i);
        return tree[p];
    }
    T query(int L, int R) {
        if (L < 0 || R >= n || L > R)
            throw std::out_of_range("Invalid interval");
        L += size;
        R += size + 1;
        for (int i = log; i >= 1; i--) {
            if (((L >> i) << i) != L) push_down(L >> i);
            if (((R >> i) << i) != R) push_down((R - 1) >> i);
        }
        T lans = e(), rans = e();
        while (L < R) {
            if (L & 1) lans = op(lans, tree[L++]);
            if (R & 1) rans = op(tree[--R], rans);
            L >>= 1;
            R >>= 1;
        }
        return op(lans, rans);
    }
};
} // namespace mystd

#endif // SEGMENTTREE_HPP