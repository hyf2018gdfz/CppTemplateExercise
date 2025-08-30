#ifndef COMMON_H
#define COMMON_H

#include <bits/stdc++.h>

using namespace std;

const int iinf = 0x3f3f3f3f;
const long long linf = 2e18;
const int mod = 998244353;

using ll = long long;
using ull = unsigned long long;

struct TestInfo {
    const char *fileName;
    const char *funcName;
    int lineNumber;
};
template <typename T, typename U>
void check_eq_impl(const T &expected, const U &actual, const TestInfo &info) {
    if (expected != actual) {
        cerr << "[FAIL] " << info.funcName << " (" << info.fileName << ":"
             << info.lineNumber << "): expected=" << expected
             << " got=" << actual << "\n";
        throw runtime_error("Test failed");
    }
}

#define CHECK_EQ(expected, actual)                                             \
    check_eq_impl((expected), (actual),                                        \
                  (TestInfo){__FILE__, __FUNCTION__, __LINE__})

#endif // COMMON_H