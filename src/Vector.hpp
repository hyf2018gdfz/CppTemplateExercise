#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <new>
#include <stdexcept>

#include "common.h"

namespace mystd::vector {

/// INFO: 完全使用 placement new/delete 进行内存管理
template <typename T>
class Vector {
private:
  size_t size_ = 0;
  size_t capacity_ = 0;
  T *data_ = nullptr;

  static constexpr size_t growth_factor = 2;

public:
  Vector() noexcept = default;
  explicit Vector(size_t count) : size_(count), capacity_(count) {
    if (capacity_ > 0) {
      data_ = static_cast<T *>(::operator new(capacity_ * sizeof(T)));
      for (size_t i = 0; i < size_; i++) {
        new (&data_[i]) T();
      }
    }
  }
  Vector(size_t count, const T &val) : size_(count), capacity_(count) {
    if (capacity_ > 0) {
      data_ = static_cast<T *>(::operator new(capacity_ * sizeof(T)));
      for (size_t i = 0; i < size_; i++) {
        new (&data_[i]) T(val);
      }
    }
  }
  Vector(const Vector &other) : size_(other.size_), capacity_(other.capacity_) {
    if (capacity_ > 0) {
      data_ = static_cast<T *>(::operator new(capacity_ * sizeof(T)));
      for (size_t i = 0; i < size_; i++) {
        new (&data_[i]) T(other.data_[i]);
      }
    }
  }
  Vector(Vector &&other) noexcept
      : size_(other.size_), capacity_(other.capacity_), data_(other.data_) {
    other.size_ = 0;
    other.capacity_ = 0;
    other.data_ = nullptr;
  }
  Vector(std::initializer_list<T> init)
      : size_(init.size()), capacity_(init.size()) {
    if (capacity_ > 0) {
      data_ = static_cast<T *>(::operator new(capacity_ * sizeof(T)));
      std::uninitialized_copy(init.begin(), init.end(), data_);
    }
  }
  ~Vector() {
    for (size_t i = 0; i < size_; i++) {
      data_[i].~T();
    }
    ::operator delete(data_);
  }

  auto operator=(const Vector &other) -> Vector & {
    if (this == &other) {
      return *this;
    }
    clear();
    size_ = other.size_;
    capacity_ = other.capacity_;
    data_ = nullptr;
    if (capacity_ > 0) {
      data_ = static_cast<T *>(::operator new(capacity_ * sizeof(T)));
      for (size_t i = 0; i < size_; i++) {
        new (&data_[i]) T(other.data_[i]);
      }
    }
    return *this;
  }
  auto operator=(Vector &&other) noexcept -> Vector & {
    if (this == &other) {
      return *this;
    }
    clear();
    size_ = other.size_;
    capacity_ = other.capacity_;
    data_ = other.data_;
    other.size_ = 0;
    other.capacity_ = 0;
    other.data_ = nullptr;
    return *this;
  }
  auto operator=(std::initializer_list<T> init) -> Vector & {
    clear();
    size_ = init.size();
    capacity_ = init.size();
    data_ = nullptr;
    if (capacity_ > 0) {
      data_ = static_cast<T *>(::operator new(capacity_ * sizeof(T)));
      std::uninitialized_copy(init.begin(), init.end(), data_);
    }
    return *this;
  }

  auto operator[](size_t ind) -> T & { return data_[ind]; }
  auto operator[](size_t ind) const -> const T & { return data_[ind]; }
  auto at(size_t ind) -> T & {
    if (ind >= size_) {
      throw std::out_of_range("Vector::at index out of range");
    }
    return data_[ind];
  }
  auto at(size_t ind) const -> const T & {
    if (ind >= size_) {
      throw std::out_of_range("Vector::at index out of range");
    }
    return data_[ind];
  }

  /// INFO: clear 清空 m_data 但不清空 m_capacity
  void clear() {
    for (size_t i = 0; i < size_; i++) {
      data_[i].~T();
    }
    size_ = 0;
  }

  /// INFO: 出于保持简洁的原因，采用万能引用的写法而非两个重载的写法
  template <typename U>
  void pushBack(U &&val) {
    if (size_ == capacity_) {
      size_t new_cap = (capacity_ == 0 ? 1 : capacity_ * growth_factor);
      reserve(new_cap);
    }
    new (&data_[size_++]) T(std::forward<U>(val));
  }
  template <typename... Args>
  auto emplaceBack(Args &&...args) -> T & {
    if (size_ == capacity_) {
      size_t new_cap = (capacity_ == 0 ? 1 : capacity_ * growth_factor);
      reserve(new_cap);
    }
    new (&data_[size_]) T(std::forward<Args>(args)...);
    return data_[size_++];
  }

  /// INFO: 以下采取了两种方法：
  /// 1. 对于插入单个元素，末尾使用 placement new，中间使用移动赋值
  /// 2. 对于插入多个元素，统一新地址 placement new，旧地址析构
  template <typename U>
  auto insert(T *loc_ptr, U &&val) -> T * {
    size_t ind = loc_ptr - data_;
    if (ind > size_) {
      throw std::out_of_range("Vector::insert index out of range");
    }
    if (size_ == capacity_) {
      size_t new_cap = (capacity_ == 0 ? 1 : capacity_ * growth_factor);
      reserve(new_cap);
    }
    if (ind < size_) {
      new (&data_[size_]) T(std::move(data_[size_ - 1]));
      for (size_t i = size_ - 1; i > ind; i--) {
        data_[i] = std::move(data_[i - 1]);
      }
    }
    data_[ind] = T(std::forward<U>(val));
    size_++;
    return data_ + ind;
  }
  template <typename U>
  auto insert(T *loc_ptr, size_t count, U &&val) -> T * {
    size_t ind = loc_ptr - data_;
    if (ind > size_) {
      throw std::out_of_range("Vector::insert index out of range");
    }
    if (size_ + count > capacity_) {
      size_t geometric_growth_cap =
          (capacity_ == 0 ? 1 : capacity_ * growth_factor);
      reserve(mystd::max(geometric_growth_cap, size_ + count));
    }
    if (ind < size_) {
      for (size_t i = size_; i > ind; i--) {
        new (&data_[i + count - 1]) T(std::move(data_[i - 1]));
        data_[i - 1].~T();
      }
    }
    for (size_t i = ind; i < ind + count; i++) {
      new (&data_[i]) T(val);
    }
    size_ += count;
    return data_ + ind;
  }
  auto insert(T *loc_ptr, std::initializer_list<T> init) -> T * {
    size_t ind = loc_ptr - data_;
    if (ind > size_) {
      throw std::out_of_range("Vector::insert index out of range");
    }
    size_t count = init.size();
    if (size_ + count > capacity_) {
      size_t geometric_growth_cap =
          (capacity_ == 0 ? 1 : capacity_ * growth_factor);
      reserve(mystd::max(geometric_growth_cap, size_ + count));
    }
    if (ind < size_) {
      for (size_t i = size_; i > ind; i--) {
        new (&data_[i + count - 1]) T(std::move(data_[i - 1]));
        data_[i - 1].~T();
      }
    }
    std::uninitialized_copy(init.begin(), init.end(), data_ + ind);
    size_ += count;
    return data_ + ind;
  }
  template <typename... Args>
  auto emplace(T *loc_ptr, Args &&...args) -> T * {
    size_t ind = loc_ptr - data_;
    if (ind > size_) {
      throw std::out_of_range("Vector::emplace index out of range");
    }
    if (size_ == capacity_) {
      size_t new_cap = (capacity_ == 0 ? 1 : capacity_ * growth_factor);
      reserve(new_cap);
    }
    if (ind < size_) {
      new (&data_[size_]) T(std::move(data_[size_ - 1]));
      for (size_t i = size_ - 1; i > ind; i--) {
        data_[i] = std::move(data_[i - 1]);
      }
    }
    data_[ind] = T(std::forward<Args>(args)...);
    size_++;
    return data_ + ind;
  }
  auto erase(T *loc_ptr) -> T * {
    size_t ind = loc_ptr - data_;
    if (ind >= size_) {
      throw std::out_of_range("Vector::erase index out of range");
    }
    for (size_t i = ind; i < size_ - 1; i++) {
      data_[i] = std::move(data_[i + 1]);
    }
    data_[--size_].~T();
    return data_ + ind;
  }

  void popBack() {
    if (size_ == 0) {
      throw std::out_of_range("Vector::pop_back called on empty vector");
    }
    data_[--size_].~T();
  }

  void swap(Vector &ano) noexcept {
    mystd::swap(this->size_, ano.size_);
    mystd::swap(this->capacity_, ano.capacity_);
    mystd::swap(this->data_, ano.data_);
  }

  auto front() -> T & {
    if (size_ == 0) {
      throw std::out_of_range("Vector::front called on empty vector");
    }
    return data_[0];
  }
  auto front() const -> const T & {
    if (size_ == 0) {
      throw std::out_of_range("Vector::front called on empty vector");
    }
    return data_[0];
  }
  auto back() -> T & {
    if (size_ == 0) {
      throw std::out_of_range("Vector::back called on empty vector");
    }
    return data_[size_ - 1];
  }
  auto back() const -> const T & {
    if (size_ == 0) {
      throw std::out_of_range("Vector::back called on empty vector");
    }
    return data_[size_ - 1];
  }
  auto begin() -> T * { return data_; }
  auto cbegin() const -> const T * { return data_; }
  auto end() -> T * { return data_ + size_; }
  auto cend() const -> const T * { return data_ + size_; }

  [[nodiscard]] auto empty() const noexcept -> bool { return size_ == 0; }
  [[nodiscard]] auto size() const noexcept -> size_t { return size_; }
  [[nodiscard]] auto capacity() const noexcept -> size_t { return capacity_; }
  void reserve(size_t new_cap) {
    if (new_cap <= capacity_) {
      return;
    }
    T *new_data = static_cast<T *>(::operator new(new_cap * sizeof(T)));
    for (size_t i = 0; i < size_; i++) {
      new (&new_data[i]) T(std::move(data_[i]));
      data_[i].~T();
    }
    ::operator delete(data_);
    data_ = new_data;
    capacity_ = new_cap;
  }
  void resize(size_t new_size) {
    if (new_size > size_) {
      reserve(new_size);
      for (size_t i = size_; i < new_size; i++) {
        new (&data_[i]) T();
      }
    } else if (new_size < size_) {
      for (size_t i = new_size; i < size_; i++) {
        data_[i].~T();
      }
    }
    size_ = new_size;
  }
  void shrinkToFit() {
    if (size_ == capacity_) {
      return;
    }
    if (size_ == 0) {
      clear();
      data_ = nullptr;
      capacity_ = 0;
      return;
    }
    T *new_data = static_cast<T *>(::operator new(size_ * sizeof(T)));
    for (size_t i = 0; i < size_; i++) {
      new (&new_data[i]) T(std::move(data_[i]));
      data_[i].~T();
    }
    ::operator delete(data_);
    data_ = new_data;
    capacity_ = size_;
  }

  auto operator==(const Vector &ano) const -> bool {
    if (size_ != ano.size_) {
      return false;
    }
    if constexpr (std::is_trivial_v<T>) {
      return std::memcmp(data_, ano.data_, size_ * sizeof(T)) == 0;
    } else {
      for (size_t i = 0; i < size_; i++) {
        if (!(data_[i] == ano.data_[i])) {
          return false;
        }
      }
      return true;
    }
  }
  auto operator!=(const Vector &ano) const -> bool { return !(*this == ano); }
};
}  // namespace mystd::vector

#endif