# `mystd::memory::UniquePtr` 函数功能

参见 [C++ Reference](https://cppreference.cn/w/cpp/memory/unique_ptr)，目前已经完整复刻了所有的成员函数。

# `mystd::memory::UniquePtr` 实现细节

## 1. 概述

本实现提供了一个独占所有权的智能指针 `UniquePtr`，包含针对单对象 `UniquePtr<T>` 和动态数组 `UniquePtr<T[]>` 的偏特化版本。
代码主要基于 **C++17** 标准实现，并利用宏 `MYSTD_HAS_CXX20` 提供了基于 **C++20** `requires` 子句的更优雅的编译期约束。

## 2. 核心底层机制

### 2.1 默认删除器 (`DefaultDeleter`)

实现包含两个版本的默认删除器，利用 SFINAE 允许派生类指针向基类指针转换的删除器拷贝构造：

* **单对象版本 `DefaultDeleter<T>**`: 重载 `operator()` 调用 `delete ptr;`。
* **数组版本 `DefaultDeleter<T[]>**`: 重载 `operator()` 调用 `delete[] ptr;`。

### 2.2 空基类优化 (EBO - `CompressedPair`)

为了避免零大小的删除器（如 `DefaultDeleter`）占用额外的内存导致智能指针体积膨胀（从 8 字节变成 16 字节），实现中设计了 `mystd::memory::internal::CompressedPair`：

* **触发条件**: 检查 `Deleter` 是否为空类 (`std::is_empty_v`) 且不是 `final` 类 (`!std::is_final_v`)。
* **启用 EBO (`true`)**: `CompressedPair` 私有继承 `Deleter`，将指针作为唯一的数据成员。
* **禁用 EBO (`false`)**: `CompressedPair` 将 `Deleter` 和 `Pointer` 都作为普通成员变量存储。

### 2.3 指针类型推导 (`PointerType`)

标准规定 `unique_ptr` 的内部指针类型可以通过 `Deleter::pointer` 自定义。这里使用了 SFINAE 技术（`std::void_t`）进行推导：

* 如果 `Deleter::Pointer` 存在，则 `Pointer` 的类型为 `Deleter::Pointer`。
* 否则，退化为原生的 `T*`。

---

## 3. 单对象版本特化 (`UniquePtr<T>`)

### 3.1 构造与析构

利用 SFINAE（`std::enable_if_t`）对构造函数进行了严格的类型和重载解析限制：

* **默认/空指针构造**: 检查 `Deleter` 是否可默认构造且不是指针类型。
* **指针构造**: 接受原生指针，值初始化删除器。
* **指针 + 删除器构造**: 区分了删除器是左值引用还是右值引用的情况（遵循 C++11 完美转发规则）。
* **移动构造 / 转换移动构造**: 允许将 `UniquePtr<Derived>` 移动构造为 `UniquePtr<Base>`。
* **拷贝构造**: 被 `delete`，确保所有权的独占性。

### 3.2 赋值运算符 (C++17 vs C++20 差异处理)

如源码注释所述，在 C++17 中使用 SFINAE 从重载决议中剔除赋值运算符比较困难（编译器可能会隐式生成非模板的赋值运算符）。

* **C++20 下**: 使用了 `requires` 子句，在模板推导阶段就能优雅地拦截非法的赋值操作。
* **C++17 下**: 退而求其次，在函数体内使用了 `static_assert`。如果类型不匹配（例如无法转换，或者 `Deleter` 不可赋值），会在实例化时触发编译期硬报错，阻止非法赋值。

### 3.3 访问器与修改器

* **修改器**: 提供了 `release()`（放弃控制权）、`reset()`（重置指针并销毁旧对象）和 `swap()`。
* **解引用**: 提供了原生的 `operator*()` 和 `operator->()`。

---

## 4. 动态数组版本特化 (`UniquePtr<T[]>`)

数组版本的实现逻辑与单对象版本基本一致，但为了防止诸如“通过基类指针数组删除派生类对象”等未定义行为（Undefined Behavior），增加了极其严格的**安全数组转换约束**。

### 4.1 安全数组转换 (`IsSafeArrayConversion`)

引入了静态检查来验证传入的指针类型 `U` 是否能安全地作为 `ElementType[]` 使用：

* 检查 `V(*)[]` （指向 `V` 数组的指针）是否能隐式转换为 `ElementType(*)[]`。
* 在此基础上衍生出三个核心检查常量：`CHECK_RESET_V`、`CHECK_CONSTRUCTOR_V` 和 `CHECK_OPERATOR_EQUAL_V`。

### 4.2 构造与赋值的严格限制

在数组版本的模板构造函数和 `reset` 函数中，大量使用了上述检查常量：

* 阻止非数组类型的指针被管理。
* 在 `operator=` 转换赋值时，除了要求是数组，还严格要求底层元素类型的匹配和安全转换，避免多态数组的安全隐患。

### 4.3 独有操作符

* **移除了** `operator*()` 和 `operator->()`，因为它们对数组没有直接意义。
* **提供了** `operator[](std::size_t)` 用于数组元素的随机访问。

