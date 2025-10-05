#ifndef BINARY_HEAP_HPP
#define BINARY_HEAP_HPP

#include "Compare.hpp"
#include "Vector.hpp"
#include "common.h"

#include <initializer_list>

namespace mystd::binary_heap {

/// INFO: 基于 mystd::Vector 实现的二叉堆，默认大根堆
template <typename T, typename Compare = mystd::compare::Less<T>>
class BinaryHeap {
private:
    vector::Vector<T> m_data;
    Compare m_comp;

    void sift_up(size_t idx) {
        while (idx > 0) {
            size_t parent = (idx - 1) / 2;
            if (!m_comp(m_data[parent], m_data[idx])) break;
            mystd::swap(m_data[parent], m_data[idx]);
            idx = parent;
        }
    }

    void sift_down(size_t idx) {
        size_t n = m_data.size();
        while (true) {
            size_t left = idx * 2 + 1;
            size_t right = idx * 2 + 2;
            size_t largest = idx;

            if (left < n && m_comp(m_data[largest], m_data[left]))
                largest = left;
            if (right < n && m_comp(m_data[largest], m_data[right]))
                largest = right;
            if (largest == idx) break;
            mystd::swap(m_data[idx], m_data[largest]);
            idx = largest;
        }
    }

public:
    explicit BinaryHeap(const Compare &comp = Compare()) :
        m_data(), m_comp(comp) {}

    BinaryHeap(std::initializer_list<T> init, const Compare &comp = Compare()) :
        m_data(init), m_comp(comp) {
        build_heap();
    }

    bool empty() const { return m_data.empty(); }
    size_t size() const { return m_data.size(); }

    const T &top() const {
        if (empty()) throw std::out_of_range("BinaryHeap::top on empty heap");
        return m_data[0];
    }

    template <typename U>
    void push(U &&value) {
        m_data.push_back(std::forward<U>(value));
        sift_up(m_data.size() - 1);
    }

    template <typename... Args>
    void emplace(Args &&...args) {
        m_data.emplace_back(std::forward<Args>(args)...);
        sift_up(m_data.size() - 1);
    }

    void pop() {
        if (empty()) throw std::out_of_range("BinaryHeap::pop on empty heap");
        mystd::swap(m_data[0], m_data[m_data.size() - 1]);
        m_data.pop_back();
        if (!empty()) sift_down(0);
    }

    void swap(BinaryHeap &ano) noexcept {
        this->m_data.swap(ano.m_data);
        mystd::swap(this->m_comp, ano.m_comp);
    }

private:
    void build_heap() {
        if (m_data.empty()) return;
        for (size_t i = (m_data.size() - 1) / 2 + 1; i > 0; --i) {
            sift_down(i - 1);
        }
    }
};

} // namespace mystd::binary_heap

#endif // BINARY_HEAP_HPP
