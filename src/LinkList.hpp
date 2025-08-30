#ifndef LINKLIST_HPP
#define LINKLIST_HPP

#include "common.h"

template <typename T>
class LinkList {
private:
    struct Node {
        T val;
        Node *nxt, *pre;

        /// INFO:
        /// 此处也可以写多参数传入的构造，但是考虑易读性，这里仍然写成单参数
        template <typename U>
        Node(U &&_val) : val(forward<U>(_val)), nxt(nullptr), pre(nullptr) {}
    };
    Node *head, *tail;
    int len;

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
    Node *getNodeAt(int index) {
        if (index < 0 || index >= len) return nullptr;
        Node *cur = (index < len / 2) ? head : tail;
        if (index < len / 2) {
            for (int i = 0; i < index; i++) cur = cur->nxt;
        } else {
            for (int i = len - 1; i > index; i--) cur = cur->pre;
        }
        return cur;
    }

public:
    int getLen() const { return len; }
    LinkList() {
        head = tail = nullptr;
        len = 0;
    }
    ~LinkList() { clear(); }
    LinkList(const LinkList &other) {
        head = tail = nullptr;
        len = 0;
        auto *cur = other.head;
        while (cur != nullptr) {
            push_back(cur->val);
            cur = cur->nxt;
        }
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
    LinkList(LinkList &&other) noexcept :
        head(other.head), tail(other.tail), len(other.len) {
        other.head = nullptr;
        other.tail = nullptr;
        other.len = 0;
    }
    LinkList &operator=(LinkList &&other) noexcept {
        if (this != &other) {
            clear();
            head = other.head;
            tail = other.tail;
            len = other.len;
            other.head = nullptr;
            other.tail = nullptr;
            other.len = 0;
        }
        return *this;
    }
    template <typename U>
    void push_back(U &&val) {
        Node *newNode = new Node(forward<U>(val));
        len++;
        if (head == nullptr) {
            head = tail = newNode;
            return;
        }
        tail->nxt = newNode;
        newNode->pre = tail;
        tail = newNode;
    }
    bool pop_back() {
        if (tail == nullptr) return false;
        len--;
        if (head == tail) {
            delete tail;
            head = tail = nullptr;
            return true;
        }
        auto *tmp = tail;
        tail = tail->pre;
        tail->nxt = nullptr;
        delete tmp;
        return true;
    }
    template <typename U>
    void push_front(U &&val) {
        Node *newNode = new Node(forward<U>(val));
        len++;
        if (head == nullptr) {
            head = tail = newNode;
            return;
        }
        head->pre = newNode;
        newNode->nxt = head;
        head = newNode;
    }
    bool pop_front() {
        if (head == nullptr) return false;
        len--;
        if (head == tail) {
            delete head;
            head = tail = nullptr;
            return true;
        }
        auto *tmp = head;
        head = head->nxt;
        head->pre = nullptr;
        delete tmp;
        return true;
    }
    bool erase(int index) {
        if (index < 0 || index >= len) return false;
        if (index == 0) return pop_front();
        if (index == len - 1) return pop_back();
        // 根据位置选择遍历方向
        auto *cur = getNodeAt(index);
        cur->pre->nxt = cur->nxt;
        cur->nxt->pre = cur->pre;
        delete cur;
        len--;
        return true;
    }
    template <typename U>
    bool insert(int index, U &&val) {
        if (index < 0 || index > len) return false;
        if (index == 0) {
            push_front(forward<U>(val));
            return true;
        }
        if (index == len) {
            push_back(forward<U>(val));
            return true;
        }
        auto *cur = getNodeAt(index);
        Node *newNode = new Node(forward<U>(val));
        newNode->pre = cur->pre;
        newNode->nxt = cur;
        cur->pre->nxt = newNode;
        cur->pre = newNode;
        len++;
        return true;
    }
    int find(const T &val) const {
        auto *cur = head;
        int index = 0;
        while (cur != nullptr) {
            if (cur->val == val) return index;
            cur = cur->nxt;
            index++;
        }
        return -1;
    }
    friend std::ostream &operator<<(std::ostream &os, const LinkList<T> &lst) {
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

#endif // LINKLIST_HPP