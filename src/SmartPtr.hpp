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
  auto getDeleter() noexcept -> Deleter& { return *this; }
  auto getDeleter() const noexcept -> const Deleter& { return *this; }
  auto getPointer() noexcept -> Pointer& { return ptr_; }
  auto getPointer() const noexcept -> const Pointer& { return ptr_; }
  CompressedPair(const Deleter& del, const Pointer& ptr)
      : Deleter(del), ptr_(ptr) {}
  CompressedPair(Deleter&& del, Pointer&& ptr)
      : Deleter(std::move(del)), ptr_(std::move(ptr)) {}
};

template <typename Deleter, typename Pointer>
class CompressedPair<Deleter, Pointer, false> {
  Deleter deleter_;
  Pointer ptr_;
  auto getDeleter() noexcept -> Deleter& { return deleter_; }
  auto getDeleter() const noexcept -> const Deleter& { return deleter_; }
  auto getPointer() noexcept -> Pointer& { return ptr_; }
  auto getPointer() const noexcept -> const Pointer& { return ptr_; }
  CompressedPair(const Deleter& del, const Pointer& ptr)
      : deleter_(del), ptr_(ptr) {}
  CompressedPair(Deleter&& del, Pointer&& ptr)
      : deleter_(std::move(del)), ptr_(std::move(ptr)) {}
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
      typename U = DeleterType,
      std::enable_if_t<
          std::is_default_constructible_v<U> && !std::is_pointer_v<U>, int> = 0>
  constexpr UniquePtr() noexcept(std::is_nothrow_default_constructible_v<U>)
      : cp_(DeleterType(), nullptr) {}

  template <
      typename U = DeleterType,
      std::enable_if_t<
          std::is_default_constructible_v<U> && !std::is_pointer_v<U>, int> = 0>
  constexpr UniquePtr(std::nullptr_t nptr) noexcept(
      std::is_nothrow_default_constructible_v<U>)
      : cp_(DeleterType(), nullptr) {}

  // constructor 2
  template <
      typename U = DeleterType,
      std::enable_if_t<
          std::is_default_constructible_v<U> && !std::is_pointer_v<U>, int> = 0>
  explicit UniquePtr(Pointer ptr) noexcept(
      std::is_nothrow_default_constructible_v<U>)
      : cp_(DeleterType(), ptr) {}

  // constructor 3a
  template <typename U = DeleterType,
            std::enable_if_t<!std::is_reference_v<U> &&
                                 std::is_constructible_v<U, const U&>,
                             int> = 0>
  UniquePtr(Pointer ptr, const DeleterType& del) noexcept(
      std::is_nothrow_copy_constructible_v<DeleterType>)
      : cp_(std::forward<decltype(del)>(del), ptr) {}

  template <
      typename U = DeleterType,
      std::enable_if_t<
          !std::is_reference_v<U> && std::is_constructible_v<U, U&&>, int> = 0>
  UniquePtr(Pointer ptr, DeleterType&& del) noexcept(
      std::is_nothrow_move_constructible_v<DeleterType>)
      : cp_(std::forward<decltype(del)>(del), ptr) {}

  // constructor 3b
  template <typename U = DeleterType,
            std::enable_if_t<
                std::is_lvalue_reference_v<U> &&
                    !std::is_const_v<std::remove_reference_t<U>> &&
                    std::is_constructible_v<U, std::remove_reference_t<U>&>,
                int> = 0>
  UniquePtr(Pointer ptr, std::remove_reference_t<DeleterType>& del) noexcept
      : cp_(std::forward<decltype(del)>(del), ptr) {}

  template <typename U = DeleterType,
            std::enable_if_t<
                std::is_reference_v<U> &&
                    !std::is_const_v<std::remove_reference_t<U>> &&
                    std::is_constructible_v<U, std::remove_reference_t<U>&&>,
                int> = 0>
  UniquePtr(Pointer ptr, std::remove_reference_t<DeleterType>&& del) = delete;

  // constructor 3c
  template <typename U = DeleterType,
            std::enable_if_t<
                std::is_lvalue_reference_v<U> &&
                    std::is_const_v<std::remove_reference_t<U>> &&
                    std::is_constructible_v<U, std::remove_reference_t<U>&>,
                int> = 0>
  UniquePtr(Pointer ptr, std::remove_reference_t<DeleterType>& del) noexcept
      : cp_(std::forward<decltype(del)>(del), ptr) {}

  template <typename U = DeleterType,
            std::enable_if_t<
                std::is_reference_v<U> &&
                    std::is_const_v<std::remove_reference_t<U>> &&
                    std::is_constructible_v<U, std::remove_reference_t<U>&&>,
                int> = 0>
  UniquePtr(Pointer ptr, std::remove_reference_t<DeleterType>&& del) = delete;
};
};  // namespace mystd::memory

#endif
