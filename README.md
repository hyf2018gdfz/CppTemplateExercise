This is a hands-on reimplementation of C++ standard library components to deeply understand modern C++ design and implementation.

本项目旨在通过手动实现 C++ 标准库的关键组件，深入探索现代 C++ 的设计思想与底层机制。

如果组件的注释中没有特殊指明，那么只保证在单线程状况下表现正常。

## 目标

1. 时空复杂度正确，没有 bug；
2. 尽可能通用；
3. （有取舍地）复刻 cpp reference 上的要求。

## 编译与运行

编译环境：C++17, CMake。使用 C++20 或许可以获得更加精准的语义

以下是一个编译示例：

```bash
$ cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug ./
$ cmake --build ./build
```

它会在 `build` 文件夹内生成集成测试入口 `mytest`，运行该可执行文件即可看到所有测试的结果，添加 `--suite` 参数可以指定要运行的测试。如：

```bash
$ ./build/mytest --suite=SegmentTree,LinkList
```

所有支持的测试可以使用参数 `--list-suites` 看到。

## Reference

[C++ Reference](https://cppreference.cn/w/cpp)

[MSVC's implementation of the C++ Standard Library](https://github.com/microsoft/STL)