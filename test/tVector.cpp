#include "Vector.hpp"
#include <cassert>

namespace TestVector {
class Tmp {
private:
    int a, b;

public:
    Tmp(int _a, int _b) : a(_a), b(_b) {}
};
} // namespace TestVector
using namespace TestVector;

void test_Vector() {
    mystd::Vector<int> vec1;
    mystd::Vector<int> vec2(5);
    mystd::Vector<int> vec3(5, 10);
    mystd::Vector<int> vec4(vec3);
    mystd::Vector<int> vec5(std::move(vec4));

    // Check sizes
    assert(vec1.size() == 0);
    assert(vec2.size() == 5);
    assert(vec3.size() == 5);
    assert(vec4.size() == 0); // moved-from object
    assert(vec5.size() == 5);

    // Check capacities
    assert(vec1.capacity() == 0);
    assert(vec2.capacity() == 5);
    assert(vec3.capacity() == 5);
    assert(vec4.capacity() == 0); // moved-from object
    assert(vec5.capacity() == 5);

    mystd::Vector<Tmp> vec6;
    vec6.push_back((Tmp){1, 2});
}