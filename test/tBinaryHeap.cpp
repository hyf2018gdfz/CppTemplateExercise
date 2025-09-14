#include "BinaryHeap.hpp"
#include "Compare.hpp"
#include "Vector.hpp"
#include <stdexcept>
#include <string>
#include <iostream>

using namespace mystd;

struct Person {
    std::string name;
    int score;

    Person(std::string n, int s) : name(std::move(n)), score(s) {}

    bool operator<(const Person &other) const { return score < other.score; }
    bool operator>(const Person &other) const { return score > other.score; }

    bool operator==(const Person &other) const {
        return score == other.score && name == other.name;
    }
};

static void test_basic_int_heap() {
    BinaryHeap<int, Greater<int>> min_heap;
    CHECK_EQ(true, min_heap.empty());

    min_heap.push(5);
    min_heap.push(3);
    min_heap.push(7);
    min_heap.push(1);
    CHECK_EQ(4u, min_heap.size());
    CHECK_EQ(1, min_heap.top());

    min_heap.pop();
    CHECK_EQ(3, min_heap.top());
    min_heap.pop();
    CHECK_EQ(5, min_heap.top());

    BinaryHeap<int, Less<int>> max_heap{1, 2, 3, 4, 5, 6};
    CHECK_EQ(6, max_heap.top());
    max_heap.pop();
    CHECK_EQ(5, max_heap.top());
    max_heap.pop();
    CHECK_EQ(4, max_heap.top());
}

static void test_person_heap() {
    BinaryHeap<Person, Greater<Person>> min_heap;
    min_heap.emplace("Alice", 90);
    min_heap.emplace("Bob", 70);
    min_heap.emplace("Charlie", 85);

    CHECK_EQ(3u, min_heap.size());
    CHECK_EQ(Person("Bob", 70), min_heap.top());

    min_heap.pop();
    CHECK_EQ(Person("Charlie", 85), min_heap.top());
}

static void test_initializer_list() {
    BinaryHeap<int, Less<int>> max_heap{3, 1, 4, 2};
    CHECK_EQ(4u, max_heap.size());
    CHECK_EQ(4, max_heap.top());
}

static void test_exceptions() {
    BinaryHeap<int, Greater<int>> h;
    bool caught = false;
    try {
        h.top();
    } catch (const std::out_of_range &) { caught = true; }
    CHECK_EQ(true, caught);

    caught = false;
    try {
        h.pop();
    } catch (const std::out_of_range &) { caught = true; }
    CHECK_EQ(true, caught);
}

void test_BinaryHeap() {
    test_basic_int_heap();
    test_person_heap();
    test_initializer_list();
    test_exceptions();
}
