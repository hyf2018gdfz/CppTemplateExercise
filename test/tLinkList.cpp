#include "LinkList.hpp"
#include "test.h"

void test_LinkList() {
    mystd::LinkList<int> list;

    // 初始状态测试
    CHECK_EQ(0, list.size());
    CHECK_EQ(false, list.pop_front());
    CHECK_EQ(false, list.pop_back());

    // push_back 测试
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    {
        std::ostringstream oss;
        oss << list;
        CHECK_EQ("[1, 2, 3]", oss.str());
    }

    // push_front 测试
    list.push_front(0);
    {
        std::ostringstream oss;
        oss << list;
        CHECK_EQ("[0, 1, 2, 3]", oss.str());
    }

    // pop_back 测试
    CHECK_EQ(true, list.pop_back());
    {
        std::ostringstream oss;
        oss << list;
        CHECK_EQ("[0, 1, 2]", oss.str());
    }

    // pop_front 测试
    CHECK_EQ(true, list.pop_front());
    {
        std::ostringstream oss;
        oss << list;
        CHECK_EQ("[1, 2]", oss.str());
    }

    // insert 测试
    CHECK_EQ(true, list.insert(1, 99));
    {
        std::ostringstream oss;
        oss << list;
        CHECK_EQ("[1, 99, 2]", oss.str());
    }

    // erase 测试
    CHECK_EQ(true, list.erase(1));
    {
        std::ostringstream oss;
        oss << list;
        CHECK_EQ("[1, 2]", oss.str());
    }

    // find 测试
    CHECK_EQ(0, list.find(1));
    CHECK_EQ(1, list.find(2));
    CHECK_EQ(-1, list.find(99));

    // 清空链表测试
    list.pop_back();
    list.pop_back();
    CHECK_EQ(true, list.empty());
    CHECK_EQ(0, list.size());
    CHECK_EQ(false, list.pop_back());
    CHECK_EQ(false, list.pop_front());
}