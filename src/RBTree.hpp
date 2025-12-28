#ifndef RBTREE_HPP
#define RBTREE_HPP

#include <cstddef>
#include <utility>

#include "Compare.hpp"

namespace mystd::rbtree {
enum NodeColor { RED, BLACK };
template <typename T>
struct RBNode {
  using NodePtr = RBNode<T>*;

  T val_;
  NodePtr left_, right_, parent_;
  NodeColor color_;

  explicit RBNode(const T& val)
      : val_(val),
        parent_(nullptr),
        left_(nullptr),
        right_(nullptr),
        color_(NodeColor::RED) {}

  static auto minimum(NodePtr node) -> NodePtr {
    if (node == nullptr) {
      return nullptr;
    }
    while (node->left_) {
      node = node->left_;
    }
    return node;
  }
  static auto maximum(NodePtr node) -> NodePtr {
    if (node == nullptr) {
      return nullptr;
    }
    while (node->right_) {
      node = node->right_;
    }
    return node;
  }

  static auto prefix(NodePtr node) -> NodePtr {
    if (node->left_ != nullptr) {
      return maximum(node->left_);
    }
    NodePtr parent = node->parent_;
    while (node == parent->left_) {
      node = parent;
      parent = parent->parent_;
    }
    // 后面构造了一个 root->parent = header、header->parent = root 的结构
    // 如果树只有一个节点，这里 node 最终会走到 parent
    // 为了避免循环，这里需要判断一下
    if (node->left_ != parent) {
      node = parent;
    }
    return node;
  }

  static auto suffix(NodePtr node) -> NodePtr {
    if (node->right_ != nullptr) {
      return minimum(node->right_);
    }
    NodePtr parent = node->parent_;
    while (node == parent->right_) {
      node = parent;
      parent = parent->parent_;
    }
    if (node->right_ != parent) {
      node = parent;
    }
    return node;
  }
};

// 节点存储的内容是 Value，可以通过 KeyOfValue 提取出比较键值，键值类型为 Key
template <typename Value, typename Key, typename KeyOfValue,
          typename Compare = mystd::compare::Less<Key>>
class RBTree {
public:
  using Node = RBNode<Value>;
  using NodePtr = Node*;
  static const bool LEFT_SON = false;
  static const bool RIGHT_SON = true;

  class Iterator;

  explicit RBTree(const Compare& comp = Compare()) : comp_(comp) {
    initHeader();
  }
  ~RBTree() = default;

  auto begin() -> Iterator { return Iterator(header_->left_); }
  auto end() -> Iterator { return Iterator(header_); }

  auto insert(const Value& val) -> std::pair<Iterator, bool> {
    auto [parent, whson] = findInsertPos(KeyOfValue()(val));
    return insertImpl(new Node(val), parent, whson);
  }
  auto insert(Value&& val) -> std::pair<Iterator, bool> {
    auto [parent, whson] = findInsertPos(KeyOfValue()(val));
    return insertImpl(new Node(std::move(val)), parent, whson);
  }
  auto insertUnique(const Value& val) -> std::pair<Iterator, bool> {
    auto [parent, whson] = findInsertPos(KeyOfValue()(val));
    auto iter = Iterator(parent);
    if (parent != header_) {
      if (whson == LEFT_SON && parent != leftmost()) {
        --iter;
        if (!comp_(KeyOfValue()(iter.node_->val_), KeyOfValue()(val))) {
          return {iter, false};
        }
      } else if (whson == RIGHT_SON) {
        if (!comp_(KeyOfValue()(iter.node_->val_, KeyOfValue()(val)))) {
          return {iter, false};
        }
      }
    }
    return insertImpl(new Node(val), parent, whson);
  }
  auto insertUnique(Value&& val) -> std::pair<Iterator, bool> {
    auto [parent, whson] = findInsertPos(KeyOfValue()(val));
    auto iter = Iterator(parent);
    if (parent != header_) {
      if (whson == LEFT_SON && parent != leftmost()) {
        --iter;
        if (!comp_(KeyOfValue()(iter.node_->val_), KeyOfValue()(val))) {
          return {iter, false};
        }
      } else if (whson == RIGHT_SON) {
        if (!comp_(KeyOfValue()(iter.node_->val_, KeyOfValue()(val)))) {
          return {iter, false};
        }
      }
    }
    return insertImpl(new Node(std::move(val)), parent, whson);
  }

private:
  NodePtr header_;
  size_t size_;
  Compare comp_;

  auto root() const -> NodePtr& { return header_->parent_; }
  auto leftmost() const -> NodePtr& { return header_->left_; }
  auto rightmost() const -> NodePtr& { return header_->right_; }

  void initHeader() {
    header_ = static_cast<NodePtr>(operator new(sizeof(Node)));
    header_->color_ = NodeColor::RED;
    header_->parent_ = nullptr;
    header_->left_ = header_;
    header_->right_ = header_;
    size_ = 0;
  }

  void rotateLeft(NodePtr node) {
    NodePtr rson = node->right_;
    node->right_ = rson->left_;
    if (rson->left_) {
      rson->left_->parent_ = node;
    }
    rson->parent_ = node->parent_;
    if (node == root()) {
      root() = rson;
    } else if (node == node->parent_->left_) {
      node->parent_->left_ = rson;
    } else {
      node->parent_->right_ = rson;
    }
    rson->left_ = node;
    node->parent_ = rson;
  }
  void rotateRight(NodePtr node) {
    NodePtr lson = node->left_;
    node->left_ = lson->right_;
    if (lson->right_) {
      lson->right_->parent_ = node;
    }
    lson->parent_ = node->parent_;
    if (node == root()) {
      root() = lson;
    } else if (node == node->parent_->left_) {
      node->parent_->left_ = lson;
    } else {
      node->parent_->right_ = lson;
    }
    lson->right_ = node;
    node->parent_ = lson;
  }

  void insertFix(NodePtr node) {
    while (node != root() && node->parent_->color_ == NodeColor::RED) {
      NodePtr parent = node->parent_;
      NodePtr g_parent = parent->parent_;
      if (parent == g_parent->left_) {
        NodePtr uncle = g_parent->right_;
        if (uncle && uncle->color_ == NodeColor::RED) {
          uncle->color_ = parent->color_ = NodeColor::BLACK;
          g_parent->color_ = NodeColor::RED;
          node = g_parent;
        } else {
          if (node == parent->right_) {
            node = parent;
            rotateLeft(node);
          }
          parent->color_ = NodeColor::BLACK;
          g_parent->color_ = NodeColor::RED;
          rotateRight(g_parent);
        }
      } else {
        NodePtr uncle = g_parent->left_;
        if (uncle && uncle->color_ == NodeColor::RED) {
          uncle->color_ = parent->color_ = NodeColor::BLACK;
          g_parent->color_ = NodeColor::RED;
          node = g_parent;
        } else {
          if (node == parent->left_) {
            node = parent;
            rotateRight(node);
          }
          parent->color_ = NodeColor::BLACK;
          g_parent->color_ = NodeColor::RED;
          rotateLeft(g_parent);
        }
      }
    }
    root()->color_ = NodeColor::BLACK;
  }
  void deleteFix() {}

  // 查找当前键应该插入的位置
  auto findInsertPos(const Key& key) -> std::pair<NodePtr, bool> {
    NodePtr node = root();
    NodePtr parent = header_;
    bool whson = LEFT_SON;

    while (node) {
      parent = node;
      whson = comp_(key, KeyOfValue()(node->val_));
      node = whson ? node->left_ : node->right_;
    }
    return {parent, whson};
  }

  auto insertImpl(NodePtr node, NodePtr parent,
                  bool whson) -> std::pair<Iterator, bool> {
    node->parent_ = parent;
    if (parent == header_) {
      root() = node;
      leftmost() = rightmost() = node;
    } else if (whson == LEFT_SON) {
      parent->left_ = node;
      if (parent == leftmost()) {
        leftmost() = node;
      }
    } else {
      parent->right_ = node;
      if (parent == rightmost()) {
        rightmost() = node;
      }
    }
    size_++;
    insertFix(node);
    return {Iterator(node), true};
  }

public:
  class Iterator {
    friend class RBTree;

  private:
    NodePtr node_;

  public:
    explicit Iterator(NodePtr node) : node_(node) {}
    auto operator*() -> Node& { return *node_; }
    auto operator*() const -> const Node& { return *node_; }
    auto operator->() -> Node* { return node_; }
    auto operator->() const -> const Node* { return node_; }

    auto operator++() -> Iterator& {
      node_ = Node::suffix(node_);
      return *this;
    }  // 前置++
    auto operator++(int) -> Iterator {
      Iterator temp(node_);
      node_ = Node::suffix(node_);
      return temp;
    }  // 后置++
    auto operator--() -> Iterator& {
      node_ = Node::prefix(node_);
      return *this;
    }  // 前置--
    auto operator--(int) -> Iterator {
      Iterator temp(node_);
      node_ = Node::prefix(node_);
      return temp;
    }  // 后置--

    auto operator==(const Iterator& other) const -> bool {
      return node_ == other.node_;
    }
    auto operator!=(const Iterator& other) const -> bool {
      return node_ != other.node_;
    }
  };

  auto find(const Key& key) -> Iterator {
    NodePtr node = root();
    while (node) {
      Key knode = KeyOfValue()(node->val_);
      if (key == knode) {
        return Iterator(node);
      }
      if (comp_(knode, key)) {
        node = node->left_;
      } else {
        node = node->right_;
      }
    }
    return Iterator(header_);
  }
};
}  // namespace mystd::rbtree

#endif