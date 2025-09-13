#include "Vector.hpp"
#include <stdexcept>
using namespace mystd;

namespace TestVector {
class Tmp {
private:
    int a, b;

public:
    Tmp(int _a, int _b) : a(_a), b(_b) {}
    bool operator==(const Tmp &o) const { return a == o.a && b == o.b; }
};
} // namespace TestVector
using namespace TestVector;

static void test_basic_int() {
    Vector<int> v;
    CHECK_EQ(true, v.empty());
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    CHECK_EQ(3u, v.size());
    CHECK_EQ(1, v[0]);
    CHECK_EQ(3, v.back());
    v.pop_back();
    CHECK_EQ(2u, v.size());
    CHECK_EQ(2, v.back());
}

static void test_ctor_and_assign() {
    Vector<int> v1(3, 7); // {7,7,7}
    CHECK_EQ(3u, v1.size());
    CHECK_EQ(7, v1[1]);

    Vector<int> v2{1, 2, 3};
    CHECK_EQ(3u, v2.size());
    CHECK_EQ(2, v2[1]);

    Vector<int> v3 = v2; // copy
    CHECK_EQ(true, v2 == v3);

    Vector<int> v4 = std::move(v2); // move
    CHECK_EQ(3u, v4.size());
    CHECK_EQ(true, v2.empty());

    Vector<int> v5;
    v5 = v4; // copy assign
    CHECK_EQ(v4, v5);

    Vector<int> v6;
    v6 = std::move(v4); // move assign
    CHECK_EQ(3u, v6.size());
    CHECK_EQ(true, v4.empty());

    Vector<int> v7;
    v7 = {9, 8, 7};
    CHECK_EQ(3u, v7.size());
    CHECK_EQ(8, v7[1]);
}

static void test_insert_erase() {
    Vector<int> v{1, 2, 3};
    v.insert(v.begin() + 1, 99); // {1,99,2,3}
    CHECK_EQ(4u, v.size());
    CHECK_EQ(99, v[1]);

    v.insert(v.begin() + 2, 2, 77); // {1,99,77,77,2,3}
    CHECK_EQ(6u, v.size());
    CHECK_EQ(77, v[2]);
    CHECK_EQ(77, v[3]);

    v.insert(v.end(), {5, 6});
    CHECK_EQ(8u, v.size());
    CHECK_EQ(6, v.back());

    v.erase(v.begin() + 1); // erase 99
    CHECK_EQ(7u, v.size());
    CHECK_EQ(77, v[1]);
}

static void test_emplace() {
    Vector<Tmp> v;
    v.emplace_back(1, 2);
    v.emplace(v.begin(), 3, 4);
    CHECK_EQ(2u, v.size());
    CHECK_EQ(Tmp(3, 4), v[0]);
    CHECK_EQ(Tmp(1, 2), v[1]);
}

static void test_capacity_resize() {
    Vector<int> v;
    v.reserve(10);
    CHECK_EQ(true, v.capacity() >= 10);
    v.push_back(42);
    v.resize(5);
    CHECK_EQ(5u, v.size());
    CHECK_EQ(42, v[0]);
    CHECK_EQ(0, v[1]); // default constructed int
    v.shrink_to_fit();
    CHECK_EQ(5u, v.capacity());
    v.clear();
    CHECK_EQ(0u, v.size());
}

static void test_exceptions() {
    Vector<int> v{1, 2, 3};
    bool caught = false;
    try {
        (void)v.at(10);
    } catch (const std::out_of_range &) { caught = true; }
    CHECK_EQ(true, caught);

    caught = false;
    try {
        Vector<int> empty;
        empty.pop_back();
    } catch (const std::out_of_range &) { caught = true; }
    CHECK_EQ(true, caught);
}

void test_Vector() {
    test_basic_int();
    test_ctor_and_assign();
    test_insert_erase();
    test_emplace();
    test_capacity_resize();
    test_exceptions();
}
