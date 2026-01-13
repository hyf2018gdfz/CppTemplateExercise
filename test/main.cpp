#include <bits/stdc++.h>

#include <iostream>

#include "test.h"

using namespace std;

int main(int argc, char *argv[]) {
  std::vector<std::string> suites;
  bool list_suites = false;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg.rfind("--suite=", 0) == 0) {
      std::string list = arg.substr(8);  // comma-separated suite names
      std::stringstream ss(list);
      std::string item;
      while (std::getline(ss, item, ',')) {
        if (!item.empty()) {
          suites.push_back(item);
        }
      }
    } else if (arg == "--list-suites") {
      list_suites = true;
    } else {
      std::cerr << "Unknown argument " << arg << "\n";
      list_suites = true;
    }
  }

  if (list_suites) {
    auto registered = TestRegistry::instance().getSuites();
    std::cout << "Registered test suites:\n";
    for (const auto &suite : registered) {
      std::cout << "- " << suite << "\n";
      auto cases = TestRegistry::instance().getCases(suite);
      for (const auto &c : cases) {
        std::cout << "    - " << c << "\n";
      }
    }
    return 0;
  }

  int failed = 0;
  if (suites.empty()) {
    failed = TestRegistry::instance().runAll();
  } else {
    failed = TestRegistry::instance().runSuites(suites);
  }

  if (failed == 0) {
    std::cout << "All tests passed.\n";
  } else {
    std::cout << failed << " test(s) failed.\n";
  }

  return failed > 0 ? 1 : 0;
}
