#include <bits/stdc++.h>

#include "testcase.h"

using namespace std;

using TestFunc = void (*)();

const string RESET = "\033[0m";
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string CYAN = "\033[36m";

// 运行单个测试的辅助函数
void run_test(const string &name, TestFunc func, int &passed, int &failed) {
  cout << CYAN << "==== Running " << name << " ====" << RESET << "\n";
  auto start = chrono::high_resolution_clock::now();
  try {
    func();
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end - start;
    cout << GREEN << "[OK] " << RESET << name << " completed in "
         << diff.count() << "s\n";
    passed++;
  } catch (exception &e) {
    cerr << RED << "[ERROR] " << RESET << name << " failed: " << e.what()
         << "\n";
    failed++;
  }
}

int main(int argc, char *argv[]) {
  map<string, TestFunc> tests = {
      {"SegmentTree", test_SegmentTree},
      {"LinkList", test_LinkList},
      {"FenwickTree", test_FenwickTree},
      {"Vector", test_Vector},
      {"BinaryHeap", test_BinaryHeap},
      {"Stack", test_Stack}
      // 新模块加到这里
  };

  vector<string> modules;

  for (int i = 1; i < argc; i++) {
    string arg = argv[i];
    if (arg.rfind("--module=", 0) == 0) {
      string list = arg.substr(9);
      stringstream ss(list);
      string item;
      while (getline(ss, item, ',')) {
        modules.push_back(item);
      }
    }
  }

  int passed_count = 0;
  int failed_count = 0;

  if (modules.empty()) {
    for (auto &p : tests) {
      run_test(p.first, p.second, passed_count, failed_count);
    }
  } else {
    for (auto &m : modules) {
      auto it = tests.find(m);
      if (it == tests.end()) {
        cerr << "Unknown module: " << m << "\n";
        continue;
      }
      run_test(it->first, it->second, passed_count, failed_count);
    }
  }

  cout << "\n==== Test Summary ====\n";
  if (failed_count == 0) {
    cout << GREEN << "All " << passed_count << " tests passed!" << RESET
         << "\n";
  } else {
    cout << GREEN << "Passed: " << passed_count << RESET << "\n";
    cout << RED << "Failed: " << failed_count << RESET << "\n";
  }
  cout << "Total: " << passed_count + failed_count << "\n";

  return failed_count > 0 ? 1 : 0;
}
