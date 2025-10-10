# `SegmentTree` 模板使用文档

## 类模板定义

```cpp
template <typename T,
          T (*op)(T, T),
          T (*e)(),
          typename F,
          T (*mapping)(F, T),
          F (*composition)(F, F),
          F (*id)()>
class SegmentTree;
```

### 模板参数含义

| 参数          | 类型               | 含义                                                 |
| ------------- | ------------------ | ---------------------------------------------------- |
| `T`           | 节点类型           | 存储在线段树节点的值（如区间和、最值等）             |
| `op`          | 函数指针 `T(T, T)` | 节点合并函数，例如加法、取最小值等                   |
| `e`           | 函数指针 `T()`     | 单位元（`op(x, e()) = x`）                           |
| `F`           | 懒标记类型         | 对节点的操作（例如加法、乘法、线性变换等）           |
| `mapping`     | 函数指针 `T(F, T)` | 将对节点的操作应用到节点上                           |
| `composition` | 函数指针 `F(F, F)` | 合并两个操作（`new_tag = composition(f_new, f_old)`) |
| `id`          | 函数指针 `F()`     | 操作的恒等元素                                       |

## 接口说明

| 方法                                     | 功能                                          | 时间复杂度  |
| ---------------------------------------- | --------------------------------------------- | ----------- |
| `SegmentTree(int n)`                     | 构造一个长度为 $n$ 的线段树（初始值为 `e()`） | $O(n)$      |
| `SegmentTree(const std::vector<T> &arr)` | 从数组初始化线段树                            | $O(n)$      |
| `void assign(int p, T val)`              | 单点赋值（替换原值）                          | $O(\log n)$ |
| `void apply(int p, F f)`                 | 单点应用操作 `f`                              | $O(\log n)$ |
| `void apply(int L, int R, F f)`          | 区间 $[L, R]$ 应用操作 `f`                    | $O(\log n)$ |
| `T query(int p)`                         | 查询单点值                                    | $O(\log n)$ |
| `T query(int L, int R)`                  | 查询区间 $[L, R]$ 的和                        | $O(\log n)$ |

## 行为说明

该模板支持**带懒标记的区间操作**，内部采用**半开区间 $[L, R+1)$ 表示**，
外部 API 对用户呈现为**闭区间 $[L, R]$**。请注意，所有操作涉及的**下标从 $0$ 开始**，例如 `query(1)` 表示计算数组第 $2$ 个元素的值。

## 使用案例

我们定义操作：

1. 对区间内每个元素执行 $x \gets a x + b \pmod{10^9+7}$
2. 求和 $\sum\limits_{i = L}^R x_i$

那么可以像如下方式定义线段树。

```cpp
const int mod = 1e9 + 7;
// 区间修改 x <- x * a + b、区间求和，答案对 mod 取模
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
node mapping(func fn, node nd) {
    return node{(nd.a * fn.a % mod + nd.size * fn.b % mod) % mod, nd.size};
}
func composition(func new_fn, func old_fn) {
    return func{new_fn.a * old_fn.a % mod,
                (new_fn.a * old_fn.b % mod + new_fn.b) % mod};
}
func id() {
    return func{1, 0};
}

vector<node> arr(5);
for (int i = 0; i < 5; i++) arr[i] = node{(i + 1), 1}; // [1, 2, 3, 4, 5]
SegmentTree<node, op, e, func, mapping, composition, id> seg(arr);
```

更为详细的使用案例请参考 `test/tSegmentTree.cpp`。