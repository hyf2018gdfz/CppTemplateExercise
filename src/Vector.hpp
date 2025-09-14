#ifndef VECTOR_HPP
#define VECTOR_HPP

#include "common.h"
#include <initializer_list>
#include <type_traits>
#include <cstring>

namespace mystd {

/// INFO: 完全使用 placement new/delete 进行内存管理
template <typename T>
class Vector {
private:
    size_t m_size = 0;
    size_t m_capacity = 0;
    T *m_data = nullptr;

    static constexpr size_t GROWTH_FACTOR = 2;

    /// INFO: 仅析构并释放 m_data，不影响 m_size 和 m_capacity
    inline void destroy_data() {
        for (size_t i = 0; i < m_size; i++) { m_data[i].~T(); }
        ::operator delete(m_data);
        m_data = nullptr;
    }

public:
    Vector() noexcept = default;
    explicit Vector(size_t count) : m_size(count), m_capacity(count) {
        if (m_capacity > 0) {
            m_data = static_cast<T *>(::operator new(m_capacity * sizeof(T)));
            for (size_t i = 0; i < m_size; i++) { new (&m_data[i]) T(); }
        }
    }
    Vector(size_t count, const T &val) : m_size(count), m_capacity(count) {
        if (m_capacity > 0) {
            m_data = static_cast<T *>(::operator new(m_capacity * sizeof(T)));
            for (size_t i = 0; i < m_size; i++) { new (&m_data[i]) T(val); }
        }
    }
    Vector(const Vector &other) :
        m_size(other.m_size), m_capacity(other.m_capacity) {
        if (m_capacity > 0) {
            m_data = static_cast<T *>(::operator new(m_capacity * sizeof(T)));
            for (size_t i = 0; i < m_size; i++) {
                new (&m_data[i]) T(other.m_data[i]);
            }
        }
    }
    Vector(Vector &&other) noexcept :
        m_size(other.m_size), m_capacity(other.m_capacity),
        m_data(other.m_data) {
        other.m_size = 0;
        other.m_capacity = 0;
        other.m_data = nullptr;
    }
    Vector(std::initializer_list<T> init) :
        m_size(init.size()), m_capacity(init.size()) {
        if (m_capacity > 0) {
            m_data = static_cast<T *>(::operator new(m_capacity * sizeof(T)));
            size_t i = 0;
            for (auto &val : init) { new (&m_data[i++]) T(val); }
        }
    }
    ~Vector() { destroy_data(); }

    Vector &operator=(const Vector &other) {
        if (this == &other) return *this;
        destroy_data();
        m_size = other.m_size;
        m_capacity = other.m_capacity;
        if (m_capacity > 0) {
            m_data = static_cast<T *>(::operator new(m_capacity * sizeof(T)));
            for (size_t i = 0; i < m_size; i++) {
                new (&m_data[i]) T(other.m_data[i]);
            }
        }
        return *this;
    }
    Vector &operator=(Vector &&other) noexcept {
        if (this == &other) return *this;
        destroy_data();
        m_size = other.m_size;
        m_capacity = other.m_capacity;
        m_data = other.m_data;
        other.m_size = 0;
        other.m_capacity = 0;
        other.m_data = nullptr;
        return *this;
    }
    Vector &operator=(std::initializer_list<T> init) {
        destroy_data();
        m_size = init.size();
        m_capacity = init.size();
        if (m_capacity > 0) {
            m_data = static_cast<T *>(::operator new(m_capacity * sizeof(T)));
            size_t i = 0;
            for (auto &val : init) { new (&m_data[i++]) T(val); }
        }
        return *this;
    }

    T &operator[](size_t pos) { return m_data[pos]; }
    const T &operator[](size_t pos) const { return m_data[pos]; }
    T &at(size_t pos) {
        if (pos >= m_size) throw std::out_of_range("Vector::at out of range");
        return m_data[pos];
    }
    const T &at(size_t pos) const {
        if (pos >= m_size) throw std::out_of_range("Vector::at out of range");
        return m_data[pos];
    }

    /// INFO: clear 清空 m_data 但不清空 m_capacity
    void clear() {
        destroy_data();
        m_size = 0;
    }

    /// INFO: 出于保持简洁的原因，采用万能引用的写法而非两个重载的写法
    template <typename U>
    void push_back(U &&val) {
        if (m_size == m_capacity) {
            size_t new_cap = (m_capacity == 0 ? 1 : m_capacity * GROWTH_FACTOR);
            reserve(new_cap);
        }
        new (&m_data[m_size++]) T(std::forward<U>(val));
    }
    template <typename... Args>
    T &emplace_back(Args &&...args) {
        if (m_size == m_capacity) {
            size_t new_cap = (m_capacity == 0 ? 1 : m_capacity * GROWTH_FACTOR);
            reserve(new_cap);
        }
        new (&m_data[m_size]) T(std::forward<Args>(args)...);
        return m_data[m_size++];
    }

    /// INFO: 以下采取了两种方法：
    /// 1. 对于插入单个元素，末尾使用 placement new，中间使用移动赋值
    /// 2. 对于插入多个元素，统一新地址 placement new，旧地址析构
    template <typename U>
    T *insert(T *loc_ptr, U &&val) {
        size_t pos = loc_ptr - m_data;
        if (pos > m_size) throw std::out_of_range("Vector::at out of range");
        if (m_size == m_capacity) {
            size_t new_cap = (m_capacity == 0 ? 1 : m_capacity * GROWTH_FACTOR);
            reserve(new_cap);
        }
        if (pos < m_size) {
            new (&m_data[m_size]) T(std::move(m_data[m_size - 1]));
            for (size_t i = m_size - 1; i > pos; i--) {
                m_data[i] = std::move(m_data[i - 1]);
            }
        }
        m_data[pos] = T(std::forward<U>(val));
        m_size++;
        return m_data + pos;
    }
    template <typename U>
    T *insert(T *loc_ptr, size_t count, U &&val) {
        size_t pos = loc_ptr - m_data;
        if (pos > m_size) throw std::out_of_range("Vector::at out of range");
        if (m_size + count > m_capacity) {
            size_t geometric_growth_cap =
                (m_capacity == 0 ? 1 : m_capacity * GROWTH_FACTOR);
            reserve(std::max(geometric_growth_cap, m_size + count));
        }
        if (pos < m_size) {
            for (size_t i = m_size; i > pos; i--) {
                new (&m_data[i + count - 1]) T(std::move(m_data[i - 1]));
                m_data[i - 1].~T();
            }
        }
        for (size_t i = pos; i < pos + count; i++) { new (&m_data[i]) T(val); }
        m_size += count;
        return m_data + pos;
    }
    T *insert(T *loc_ptr, std::initializer_list<T> init) {
        size_t pos = loc_ptr - m_data;
        if (pos > m_size) throw std::out_of_range("Vector::at out of range");
        size_t count = init.size();
        if (m_size + count > m_capacity) {
            size_t geometric_growth_cap =
                (m_capacity == 0 ? 1 : m_capacity * GROWTH_FACTOR);
            reserve(std::max(geometric_growth_cap, m_size + count));
        }
        if (pos < m_size) {
            for (size_t i = m_size; i > pos; i--) {
                new (&m_data[i + count - 1]) T(std::move(m_data[i - 1]));
                m_data[i - 1].~T();
            }
        }
        size_t i = pos;
        for (auto &val : init) { new (&m_data[i++]) T(val); }
        m_size += count;
        return m_data + pos;
    }
    template <typename... Args>
    T *emplace(T *loc_ptr, Args &&...args) {
        size_t pos = loc_ptr - m_data;
        if (pos > m_size) throw std::out_of_range("Vector::at out of range");
        if (m_size == m_capacity) {
            size_t new_cap = (m_capacity == 0 ? 1 : m_capacity * GROWTH_FACTOR);
            reserve(new_cap);
        }
        if (pos < m_size) {
            new (&m_data[m_size]) T(std::move(m_data[m_size - 1]));
            for (size_t i = m_size - 1; i > pos; i--) {
                m_data[i] = std::move(m_data[i - 1]);
            }
        }
        m_data[pos] = T(std::forward<Args>(args)...);
        m_size++;
        return m_data + pos;
    }
    T *erase(T *loc_ptr) {
        size_t pos = loc_ptr - m_data;
        if (pos >= m_size) throw std::out_of_range("Vector::at out of range");
        for (size_t i = pos; i < m_size - 1; i++) {
            m_data[i] = std::move(m_data[i + 1]);
        }
        m_data[--m_size].~T();
        return m_data + pos;
    }

    void pop_back() {
        if (m_size == 0) throw std::out_of_range("Vector::at out of range");
        m_data[--m_size].~T();
    }

    void swap(Vector &ano) noexcept {
        mystd::swap(this->m_size, ano.m_size);
        mystd::swap(this->m_capacity, ano.m_capacity);
        mystd::swap(this->m_data, ano.m_data);
    }

    T &front() {
        if (m_size == 0) throw std::out_of_range("Vector::at out of range");
        return m_data[0];
    }
    const T &front() const {
        if (m_size == 0) throw std::out_of_range("Vector::at out of range");
        return m_data[0];
    }
    T &back() {
        if (m_size == 0) throw std::out_of_range("Vector::at out of range");
        return m_data[m_size - 1];
    }
    const T &back() const {
        if (m_size == 0) throw std::out_of_range("Vector::at out of range");
        return m_data[m_size - 1];
    }
    T *begin() { return m_data; }
    const T *cbegin() const { return m_data; }
    T *end() { return m_data + m_size; }
    const T *cend() const { return m_data + m_size; }

    bool empty() const noexcept { return m_size == 0; }
    size_t size() const noexcept { return m_size; }
    size_t capacity() const noexcept { return m_capacity; }
    void reserve(size_t new_cap) {
        if (new_cap <= m_capacity) return;
        T *new_data = static_cast<T *>(::operator new(new_cap * sizeof(T)));
        for (size_t i = 0; i < m_size; i++) {
            new (&new_data[i]) T(std::move(m_data[i]));
            m_data[i].~T();
        }
        ::operator delete(m_data);
        m_data = new_data;
        m_capacity = new_cap;
    }
    void resize(size_t new_size) {
        if (new_size > m_size) {
            reserve(new_size);
            for (size_t i = m_size; i < new_size; i++) { new (&m_data[i]) T(); }
        } else if (new_size < m_size) {
            for (size_t i = new_size; i < m_size; i++) { m_data[i].~T(); }
        }
        m_size = new_size;
    }
    void shrink_to_fit() {
        if (m_size == m_capacity) return;
        if (m_size == 0) {
            destroy_data();
            m_capacity = 0;
            return;
        }
        T *new_data = static_cast<T *>(::operator new(m_size * sizeof(T)));
        for (size_t i = 0; i < m_size; i++) {
            new (&new_data[i]) T(std::move(m_data[i]));
            m_data[i].~T();
        }
        ::operator delete(m_data);
        m_data = new_data;
        m_capacity = m_size;
    }

    bool operator==(const Vector &ano) const {
        if (m_size != ano.m_size) return false;
        if constexpr (std::is_trivial_v<T>) {
            return std::memcmp(m_data, ano.m_data, m_size * sizeof(T)) == 0;
        } else {
            for (size_t i = 0; i < m_size; i++) {
                if (!(m_data[i] == ano.m_data[i])) return false;
            }
            return true;
        }
    }
    bool operator!=(const Vector &ano) const { return !(*this == ano); }
};
} // namespace mystd

#endif