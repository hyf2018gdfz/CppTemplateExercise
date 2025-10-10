这是我仿照 AtCoder 的模板库自己写的模板库，就当是模板练习了。

如果模板的开头不特殊指明，那么只保证在单线程状况下表现正常。

## 目标

1. 复杂度正确，没有 bug。

2. 尽可能通用。

## 编译与运行

编译环境：C++17, CMake。

以下是一个编译示例：

```bash
$ cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug ./
$ cmake --build ./build
```

它会在 `build` 文件夹内生成集成测试入口 `mytest`，运行该可执行文件即可看到所有测试的结果，添加 `--module` 参数可以指定要运行的测试。如：

```bash
$ ./build/mytest --module=SegmentTree,LinkList
```

所有支持的测试可以在 `test/main.cpp` 中看到。