#ifndef SET_HPP
#define SET_HPP

#include <cassert>
#include <memory>
#include <utility>

#include "Compare.hpp"

namespace mystd::set {

// SplaySet: a set-like container implemented with a splay tree.
// - Unique keys (no duplicates).
// - Bidirectional iterator.
// - Compare is a callable type with signature bool(const T&, const T&).
template <typename T, typename Compare = mystd::compare::Less<T>>
class SplaySet {
private:
  struct Node;

public:
  // Forward declarations for iterator types.
  class Iterator;
  class ConstIterator;

  SplaySet() : root_(nullptr), size_(0), comp_() {}
  explicit SplaySet(Compare Comp) : root_(nullptr), size_(0), comp_(Comp) {}

  ~SplaySet() { Clear(); }

  // No copy or assignment for simplicity (can be added later).
  SplaySet(const SplaySet &) = delete;
  SplaySet &operator=(const SplaySet &) = delete;

  // Move semantics (shallow move).
  SplaySet(SplaySet &&other) noexcept
      : root_(other.root_), size_(other.size_), comp_(std::move(other.comp_)) {
    other.root_ = nullptr;
    other.size_ = 0;
  }
  SplaySet &operator=(SplaySet &&other) noexcept {
    if (this != &other) {
      Clear();
      root_ = other.root_;
      size_ = other.size_;
      comp_ = std::move(other.comp_);
      other.root_ = nullptr;
      other.size_ = 0;
    }
    return *this;
  }

  // Basic observers
  bool empty() const noexcept { return size_ == 0; }
  std::size_t size() const noexcept { return size_; }
  Compare value_comp() const { return comp_; }

  // Insert: returns pair<iterator, bool> like std::set
  std::pair<Iterator, bool> insert(const T &key) {
    if (!root_) {
      root_ = NewNode(key);
      ++size_;
      return {Iterator(root_, this), true};
    }

    Node *x = root_;
    while (true) {
      if (!comp_(x->key, key) && !comp_(key, x->key)) {
        // equal
        Splay(x);
        return {Iterator(root_, this), false};
      }
      int dir = comp_(x->key, key) ? 1 : 0;  // 1 -> go right, 0 -> go left
      if (!x->child[dir]) {
        Node *n = NewNode(key);
        x->child[dir] = n;
        n->parent = x;
        Splay(n);
        ++size_;
        return {Iterator(root_, this), true};
      }
      x = x->child[dir];
    }
  }

  // Erase by key: returns number of elements erased (0 or 1)
  std::size_t erase(const T &key) {
    Node *x = FindNode(key);
    if (!x) return 0;
    Splay(x);
    // now x == root_
    if (!root_->child[0]) {
      Node *r = root_->child[1];
      DeleteNode(root_);
      root_ = r;
      if (root_) root_->parent = nullptr;
    } else {
      Node *left_sub = root_->child[0];
      // detach left_sub and find its maximum
      left_sub->parent = nullptr;
      Node *max_left = left_sub;
      while (max_left->child[1]) max_left = max_left->child[1];
      // splay max_left in left_sub tree to become new root
      Splay(max_left, /*target_parent=*/nullptr, left_sub);
      // attach right subtree
      max_left->child[1] = root_->child[1];
      if (max_left->child[1]) max_left->child[1]->parent = max_left;
      DeleteNode(root_);
      root_ = max_left;
      root_->parent = nullptr;
    }
    --size_;
    return 1;
  }

  // Find: returns iterator to element or end()
  Iterator find(const T &key) {
    Node *x = FindNode(key);
    if (!x) return end();
    Splay(x);
    if (!comp_(x->key, key) && !comp_(key, x->key)) {
      return Iterator(root_, this);
    }
    return end();
  }

  ConstIterator find(const T &key) const {
    Node *x = const_cast<SplaySet *>(this)->FindNode(key);
    if (!x) return cend();
    const_cast<SplaySet *>(this)->Splay(x);
    if (!comp_(x->key, key) && !comp_(key, x->key)) {
      return ConstIterator(root_, this);
    }
    return cend();
  }

  bool contains(const T &key) { return find(key) != end(); }

  void Clear() noexcept {
    DestroySubtree(root_);
    root_ = nullptr;
    size_ = 0;
  }

  // lower_bound: first element not less than key
  Iterator lower_bound(const T &key) {
    Node *x = root_;
    Node *candidate = nullptr;
    while (x) {
      if (!comp_(x->key, key)) {  // x->key >= key
        candidate = x;
        x = x->child[0];
      } else {
        x = x->child[1];
      }
    }
    if (!candidate) return end();
    Splay(candidate);
    return Iterator(root_, this);
  }

  Iterator upper_bound(const T &key) {
    Node *x = root_;
    Node *candidate = nullptr;
    while (x) {
      if (comp_(key, x->key)) {  // x->key > key
        candidate = x;
        x = x->child[0];
      } else {
        x = x->child[1];
      }
    }
    if (!candidate) return end();
    Splay(candidate);
    return Iterator(root_, this);
  }

  Iterator begin() {
    if (!root_) return end();
    Node *x = root_;
    while (x->child[0]) x = x->child[0];
    Splay(x);
    return Iterator(root_, this);
  }

  Iterator end() { return Iterator(nullptr, this); }

  ConstIterator begin() const { return cbegin(); }
  ConstIterator end() const { return cend(); }
  ConstIterator cbegin() const {
    if (!root_) return cend();
    Node *x = root_;
    while (x->child[0]) x = x->child[0];
    const_cast<SplaySet *>(this)->Splay(x);
    return ConstIterator(root_, this);
  }
  ConstIterator cend() const { return ConstIterator(nullptr, this); }

  // Debug helper (not part of std::set): validate invariants. Only in debug
  // build.
  bool ValidateBSTInvariant() const {
#ifndef NDEBUG
    return ValidateBST(root_);
#else
    return true;
#endif
  }

  // Iterator classes
  class Iterator {
  public:
    using value_type = T;
    using reference = T &;
    using pointer = T *;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    Iterator() : node_(nullptr), owner_(nullptr) {}
    reference operator*() const { return node_->key; }
    pointer operator->() const { return std::addressof(node_->key); }

    Iterator &operator++() {
      node_ = owner_->Successor(node_);
      return *this;
    }
    Iterator operator++(int) {
      Iterator tmp = *this;
      ++*this;
      return tmp;
    }
    Iterator &operator--() {
      if (!node_) {
        // end() -> last element
        node_ = owner_->MaxNode(owner_->root_);
      } else {
        node_ = owner_->Predecessor(node_);
      }
      return *this;
    }
    Iterator operator--(int) {
      Iterator tmp = *this;
      --*this;
      return tmp;
    }

    bool operator==(const Iterator &o) const {
      return node_ == o.node_ && owner_ == o.owner_;
    }
    bool operator!=(const Iterator &o) const { return !(*this == o); }

  private:
    friend class SplaySet;
    Node *node_;
    SplaySet *owner_;
    explicit Iterator(Node *n, SplaySet *o) : node_(n), owner_(o) {}
  };

  class ConstIterator {
  public:
    using value_type = T;
    using reference = const T &;
    using pointer = const T *;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    ConstIterator() : node_(nullptr), owner_(nullptr) {}
    ConstIterator(const Iterator &it) : node_(it.node_), owner_(it.owner_) {}
    reference operator*() const { return node_->key; }
    pointer operator->() const { return std::addressof(node_->key); }

    ConstIterator &operator++() {
      node_ = owner_->Successor(node_);
      return *this;
    }
    ConstIterator operator++(int) {
      ConstIterator tmp = *this;
      ++*this;
      return tmp;
    }
    ConstIterator &operator--() {
      if (!node_) {
        node_ = owner_->MaxNode(owner_->root_);
      } else {
        node_ = owner_->Predecessor(node_);
      }
      return *this;
    }
    ConstIterator operator--(int) {
      ConstIterator tmp = *this;
      --*this;
      return tmp;
    }

    bool operator==(const ConstIterator &o) const {
      return node_ == o.node_ && owner_ == o.owner_;
    }
    bool operator!=(const ConstIterator &o) const { return !(*this == o); }

  private:
    friend class SplaySet;
    Node *node_;
    const SplaySet *owner_;
    explicit ConstIterator(Node *n, const SplaySet *o) : node_(n), owner_(o) {}
  };

private:
  struct Node {
    explicit Node(const T &k) : key(k), parent(nullptr) {
      child[0] = child[1] = nullptr;
    }
    T key;
    Node *parent;
    Node *child[2];
  };

  // allocate node
  Node *NewNode(const T &key) {
    Node *n = new Node(key);
    n->child[0] = n->child[1] = nullptr;
    n->parent = nullptr;
    return n;
  }

  void DeleteNode(Node *n) noexcept { delete n; }

  // Destroy subtree recursively
  void DestroySubtree(Node *x) noexcept {
    if (!x) return;
    DestroySubtree(x->child[0]);
    DestroySubtree(x->child[1]);
    DeleteNode(x);
  }

  // BST find (does not splay)
  Node *FindNode(const T &key) const {
    Node *x = root_;
    while (x) {
      if (!comp_(x->key, key) && !comp_(key, x->key)) return x;
      int dir = comp_(x->key, key) ? 1 : 0;
      if (!x->child[dir]) break;
      x = x->child[dir];
    }
    return nullptr;
  }

  // Helper: find node that would be parent-insertion position (used
  // internally)
  Node *FindLowerBoundNode(const T &key) const {
    Node *x = root_;
    Node *candidate = nullptr;
    while (x) {
      if (!comp_(x->key, key)) {
        candidate = x;
        x = x->child[0];
      } else {
        x = x->child[1];
      }
    }
    return candidate;
  }

  // Rotate node x up (single rotation)
  void Rotate(Node *x) {
    Node *p = x->parent;
    assert(p != nullptr);
    Node *g = p->parent;
    int dir_x = (p->child[1] == x);
    Node *b = x->child[dir_x ^ 1];

    // attach b to p
    p->child[dir_x] = b;
    if (b) b->parent = p;

    // attach p as x's child opposite side
    x->child[dir_x ^ 1] = p;
    p->parent = x;

    // attach x to g
    x->parent = g;
    if (g) {
      if (g->child[0] == p)
        g->child[0] = x;
      else
        g->child[1] = x;
    } else {
      // x will become root
    }
  }

  // Splay x to become child of target_parent (if target_parent == nullptr ->
  // become root). Optional subtree_root parameter allows splaying inside a
  // detached subtree (used in erase).
  void Splay(Node *x, Node *target_parent = nullptr,
             Node *subtree_root = nullptr) {
    if (!x) return;
    Node *root_ptr = subtree_root ? subtree_root : root_;
    while (x->parent != target_parent) {
      Node *p = x->parent;
      Node *g = p->parent;
      if (g == target_parent) {
        // Zig
        Rotate(x);
      } else {
        bool zigzig = ((g->child[0] == p && p->child[0] == x) ||
                       (g->child[1] == p && p->child[1] == x));
        if (zigzig) {
          Rotate(p);
          Rotate(x);
        } else {
          Rotate(x);
          Rotate(x);
        }
      }
    }
    // If we splayed inside the main tree, update root_
    if (!subtree_root) {
      root_ = x;
      root_->parent = nullptr;
    } else {
      // if splaying inside a detached subtree, ensure its parent pointer
      // is unchanged nothing to do: callers will attach subtree
      // accordingly
    }
  }

  // Successor and predecessor helpers
  Node *MinNode(Node *x) const {
    if (!x) return nullptr;
    while (x->child[0]) x = x->child[0];
    return x;
  }
  Node *MaxNode(Node *x) const {
    if (!x) return nullptr;
    while (x->child[1]) x = x->child[1];
    return x;
  }

  Node *Successor(Node *x) const {
    if (!x) return nullptr;
    if (x->child[1]) return MinNode(x->child[1]);
    Node *p = x->parent;
    while (p && x == p->child[1]) {
      x = p;
      p = p->parent;
    }
    return p;
  }

  Node *Predecessor(Node *x) const {
    if (!x) return nullptr;
    if (x->child[0]) return MaxNode(x->child[0]);
    Node *p = x->parent;
    while (p && x == p->child[0]) {
      x = p;
      p = p->parent;
    }
    return p;
  }

#ifndef NDEBUG
  // For debug builds: validate BST property throughout subtree
  bool ValidateBST(Node *x) const {
    if (!x) return true;
    if (x->child[0]) {
      if (comp_(x->key, x->child[0]->key))
        return false;  // child[0] must be < x
      if (!ValidateBST(x->child[0])) return false;
    }
    if (x->child[1]) {
      if (comp_(x->child[1]->key, x->key))
        return false;  // child[1] must be > x
      if (!ValidateBST(x->child[1])) return false;
    }
    return true;
  }
#endif

  // Root pointer and size
  Node *root_;
  std::size_t size_;
  Compare comp_;
};

}  // namespace mystd::set

#endif