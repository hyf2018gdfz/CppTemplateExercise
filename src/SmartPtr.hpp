#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

namespace mystd::memory {
template <typename T>
struct DefaultDeleter {
  constexpr DefaultDeleter() noexcept = default;
  template <typename U>
  DefaultDeleter(const DefaultDeleter<U>& d) noexcept {}
  void operator()(T* ptr) const noexcept { delete ptr; }
};
template <typename T>
struct DefaultDeleter<T[]> {
  constexpr DefaultDeleter() noexcept = default;
  template <typename U>
  DefaultDeleter(const DefaultDeleter<U[]>& d) noexcept {}
  void operator()(T* ptr) const noexcept { delete[] ptr; }
};

namespace mystd::memory::internal {
// 空基类优化
template <typename Deleter, typename Pointer,
          bool is_EBO = std::is_empty_v<Deleter> && !std::is_final_v<Deleter>>
class CompressedPair;

template <typename Deleter, typename Pointer>
class CompressedPair<Deleter, Pointer, true> : private Deleter {
  Pointer ptr_;

public:
  auto getDeleter() noexcept -> Deleter& { return *this; }
  auto getDeleter() const noexcept -> const Deleter& { return *this; }
  auto getPointer() noexcept -> Pointer& { return ptr_; }
  auto getPointer() const noexcept -> const Pointer& { return ptr_; }
  template <typename T1, typename T2>
  CompressedPair(T1&& del, T2&& ptr)
      : Deleter(std::forward<T1>(del)), ptr_(std::forward<T2>(ptr)) {}
};

template <typename Deleter, typename Pointer>
class CompressedPair<Deleter, Pointer, false> {
  Deleter deleter_;
  Pointer ptr_;

public:
  auto getDeleter() noexcept -> Deleter& { return deleter_; }
  auto getDeleter() const noexcept -> const Deleter& { return deleter_; }
  auto getPointer() noexcept -> Pointer& { return ptr_; }
  auto getPointer() const noexcept -> const Pointer& { return ptr_; }
  template <typename T1, typename T2>
  CompressedPair(T1&& del, T2&& ptr)
      : deleter_(std::forward<T1>(del)), ptr_(std::forward<T2>(ptr)) {}
};
};  // namespace mystd::memory::internal

template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
private:
  template <typename D, typename = void>
  struct PointerType {
    using Type = T*;
  };
  template <typename D>
  struct PointerType<D, std::void_t<typename D::pointer>> {
    using Type = typename D::pointer;
  };

public:
  using Pointer = typename PointerType<std::remove_reference_t<Deleter>>::Type;
  using ElementType = T;
  using DeleterType = Deleter;

private:
  mystd::memory::internal::CompressedPair<DeleterType, Pointer> cp_;

public:
  // constructor 1
  template <
      typename U = Deleter,
      std::enable_if_t<
          std::is_default_constructible_v<U> && !std::is_pointer_v<U>, int> = 0>
  constexpr UniquePtr() noexcept(std::is_nothrow_default_constructible_v<U>)
      : cp_(Deleter(), nullptr) {}

  template <
      typename U = Deleter,
      std::enable_if_t<
          std::is_default_constructible_v<U> && !std::is_pointer_v<U>, int> = 0>
  constexpr UniquePtr(std::nullptr_t nptr) noexcept(
      std::is_nothrow_default_constructible_v<U>)
      : cp_(Deleter(), nullptr) {}

  // constructor 2
  template <
      typename U = Deleter,
      std::enable_if_t<
          std::is_default_constructible_v<U> && !std::is_pointer_v<U>, int> = 0>
  explicit UniquePtr(Pointer ptr) noexcept(
      std::is_nothrow_default_constructible_v<U>)
      : cp_(Deleter(), ptr) {}

  // constructor 3a
  template <typename U = Deleter,
            std::enable_if_t<!std::is_reference_v<U> &&
                                 std::is_constructible_v<U, const U&>,
                             int> = 0>
  UniquePtr(Pointer ptr, const Deleter& del) noexcept(
      std::is_nothrow_copy_constructible_v<Deleter>)
      : cp_(std::forward<decltype(del)>(del), ptr) {}

  template <
      typename U = Deleter,
      std::enable_if_t<
          !std::is_reference_v<U> && std::is_constructible_v<U, U&&>, int> = 0>
  UniquePtr(Pointer ptr, Deleter&& del) noexcept(
      std::is_nothrow_move_constructible_v<Deleter>)
      : cp_(std::forward<decltype(del)>(del), ptr) {}

  // constructor 3b and 3c
  template <typename U = Deleter,
            std::enable_if_t<
                std::is_lvalue_reference_v<U> &&
                    std::is_constructible_v<U, std::remove_reference_t<U>&>,
                int> = 0>
  UniquePtr(Pointer ptr, std::remove_reference_t<Deleter>& del) noexcept
      : cp_(std::forward<decltype(del)>(del), ptr) {}

  template <typename U = Deleter,
            std::enable_if_t<std::is_lvalue_reference_v<U>, int> = 0>
  UniquePtr(Pointer ptr, std::remove_reference_t<Deleter>&& del) = delete;

  // constructor 5
  template <typename U = Deleter,
            std::enable_if_t<std::is_move_constructible_v<U>, int> = 0>
  UniquePtr(UniquePtr&& uptr) noexcept(
      std::is_reference_v<Deleter> ||
      std::is_nothrow_move_constructible_v<Deleter>)
      : cp_(std::move(uptr.getDeleter()), uptr.release()) {}

  // constructor 6
  template <
      typename T1, typename D1,
      std::enable_if_t<
          std::is_convertible_v<typename UniquePtr<T1, D1>::Pointer, Pointer> &&
              !std::is_array_v<T1> &&
              (std::is_reference_v<Deleter>
                   ? std::is_same_v<Deleter, D1>
                   : std::is_convertible_v<D1, Deleter>),
          int> = 0>
  UniquePtr(UniquePtr<T1, D1>&& uptr) noexcept
      : cp_(std::forward<D1>(uptr.getDeleter()), uptr.release()) {}

  // constructor 7
  UniquePtr(const UniquePtr&) = delete;

  ~UniquePtr() {
    if (get() != nullptr) {
      getDeleter()(get());
    }
  }

  auto release() noexcept -> Pointer {
    Pointer old_ptr = cp_.getPointer();
    cp_.getPointer() = nullptr;
    return old_ptr;
  }

  auto get() const noexcept -> Pointer { return cp_.getPointer(); }
  auto getDeleter() noexcept -> Deleter& { return cp_.getDeleter(); }
  auto getDeleter() const noexcept -> const Deleter& {
    return cp_.getDeleter();
  }
};
};  // namespace mystd::memory

#endif
