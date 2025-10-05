#ifndef LINKLIST_HPP
#define LINKLIST_HPP

#include <cstddef>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace mystd::linklist {

template <typename T>
class LinkList {
private:
    struct Node {
        T val;
        Node *nxt, *pre;

        /// INFO:
        /// 此处也可以写多参数传入的构造，但是考虑易读性，这里仍然写成单参数
        template <typename U>
        Node(U &&_val) :
            val(std::forward<U>(_val)), nxt(nullptr), pre(nullptr) {}
    };
    Node *head, *tail;
    size_t len;

    Node *getNodeAt(size_t index) {
        if (index >= len) return nullptr;
        Node *cur;
        if (index < len / 2) {
            cur = head;
            for (size_t i = 0; i < index; i++) cur = cur->nxt;
        } else {
            cur = tail;
            for (size_t i = len - 1; i > index; i--) cur = cur->pre;
        }
        return cur;
    }
    const Node *getNodeAt(size_t index) const {
        if (index >= len) return nullptr;
        const Node *cur;
        if (index < len / 2) {
            cur = head;
            for (size_t i = 0; i < index; i++) cur = cur->nxt;
        } else {
            cur = tail;
            for (size_t i = len - 1; i > index; i--) cur = cur->pre;
        }
        return cur;
    }

public:
    bool empty() const noexcept { return len == 0; }
    size_t size() const noexcept { return len; }
    ptrdiff_t ssize() const noexcept { return static_cast<ptrdiff_t>(len); }

    LinkList() noexcept : head(nullptr), tail(nullptr), len(0) {}
    ~LinkList() { clear(); }
    LinkList(const LinkList &other) : head(nullptr), tail(nullptr), len(0) {
        auto *cur = other.head;
        while (cur != nullptr) {
            push_back(cur->val);
            cur = cur->nxt;
        }
    }
    LinkList(LinkList &&other) noexcept :
        head(other.head), tail(other.tail), len(other.len) {
        other.head = nullptr;
        other.tail = nullptr;
        other.len = 0;
    }

    LinkList &operator=(const LinkList &other) {
        if (this == &other) return *this;
        clear();
        auto *cur = other.head;
        while (cur != nullptr) {
            push_back(cur->val);
            cur = cur->nxt;
        }
        return *this;
    }
    LinkList &operator=(LinkList &&other) noexcept {
        if (this == &other) return *this;
        clear();
        head = other.head;
        tail = other.tail;
        len = other.len;
        other.head = nullptr;
        other.tail = nullptr;
        other.len = 0;
        return *this;
    }

    T &operator[](size_t pos) {
        Node *node = getNodeAt(pos);
        if (node == nullptr) {
            throw std::out_of_range("LinkList::operator[] index out of range");
        }
        return node->val;
    }
    const T &operator[](size_t pos) const {
        const Node *node = getNodeAt(pos);
        if (node == nullptr) {
            throw std::out_of_range("LinkList::operator[] index out of range");
        }
        return node->val;
    }

    void clear() {
        auto cur = head;
        while (cur != nullptr) {
            auto tmp = cur->nxt;
            delete cur;
            cur = tmp;
        }
        head = tail = nullptr;
        len = 0;
    }

    template <typename U>
    void push_back(U &&val) {
        Node *newNode = new Node(std::forward<U>(val));
        len++;
        if (head == nullptr) {
            head = tail = newNode;
            return;
        }
        tail->nxt = newNode;
        newNode->pre = tail;
        tail = newNode;
    }
    template <typename U>
    void push_front(U &&val) {
        Node *newNode = new Node(std::forward<U>(val));
        len++;
        if (head == nullptr) {
            head = tail = newNode;
            return;
        }
        head->pre = newNode;
        newNode->nxt = head;
        head = newNode;
    }

    void pop_back() {
        if (empty()) {
            throw std::out_of_range("LinkList::pop_back called on empty list");
        }
        len--;
        if (head == tail) {
            delete tail;
            head = tail = nullptr;
            return;
        }
        auto *tmp = tail;
        tail = tail->pre;
        tail->nxt = nullptr;
        delete tmp;
    }
    void pop_front() {
        if (empty()) {
            throw std::out_of_range("LinkList::pop_front called on empty list");
        }
        len--;
        if (head == tail) {
            delete head;
            head = tail = nullptr;
            return;
        }
        auto *tmp = head;
        head = head->nxt;
        head->pre = nullptr;
        delete tmp;
    }

    void erase(size_t index) {
        if (index >= len) {
            throw std::out_of_range("LinkList::erase index out of range");
        }
        if (index == 0) return pop_front();
        if (index == len - 1) return pop_back();

        auto *cur = getNodeAt(index);
        cur->pre->nxt = cur->nxt;
        cur->nxt->pre = cur->pre;
        delete cur;
        len--;
    }

    template <typename U>
    void insert(size_t index, U &&val) {
        if (index > len) {
            throw std::out_of_range("LinkList::insert index out of range");
        }
        if (index == 0) return push_front(std::forward<U>(val));
        if (index == len) return push_back(std::forward<U>(val));

        auto *cur = getNodeAt(index);
        Node *newNode = new Node(std::forward<U>(val));
        newNode->pre = cur->pre;
        newNode->nxt = cur;
        cur->pre->nxt = newNode;
        cur->pre = newNode;
        len++;
    }

    ptrdiff_t find(const T &val) const {
        auto *cur = head;
        ptrdiff_t index = 0;
        while (cur != nullptr) {
            if (cur->val == val) return index;
            cur = cur->nxt;
            index++;
        }
        return -1;
    }

    friend std::ostream &operator<<(std::ostream &os, const LinkList &lst) {
        os << "[";
        auto *cur = lst.head;
        while (cur != nullptr) {
            os << cur->val;
            if (cur->nxt != nullptr) os << ", ";
            cur = cur->nxt;
        }
        os << "]";
        return os;
    }
};
} // namespace mystd::linklist

#endif // LINKLIST_HPP