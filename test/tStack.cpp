#include <stack>
#include <stdexcept>

#include "Stack.hpp"
#include "test.h"
#include "testcase.h"

using mystd::stack::Stack;
using std::stack;

void test_Stack() {
  RandomGenerator gen;
  const int query_times = 100000;
  const int num_range = 100000;
  Stack<int> stk({1, 2, 3, 4, 5});
  stack<int> ref({1, 2, 3, 4, 5});
  for (int t = 0; t < query_times; t++) {
    int opt = gen.uniform_int(0, 4);
    switch (opt) {
      case 0: {
        CHECK_EQ(ref.empty(), stk.empty());
        break;
      }
      case 1: {
        CHECK_EQ(ref.size(), stk.size());
        break;
      }
      case 2: {
        if (stk.empty()) {
          EXPECT_THROW(stk.top(), std::out_of_range);
        } else {
          CHECK_EQ(ref.top(), stk.top());
        }
        break;
      }
      case 3: {
        if (stk.empty()) {
          EXPECT_THROW(stk.pop(), std::out_of_range);
        } else {
          ref.pop();
          stk.pop();
        }
        break;
      }
      case 4: {
        int val = gen.uniform_int(0, num_range);
        ref.push(val);
        stk.push(val);
        break;
      }
      default:
        break;
    }
  }
}