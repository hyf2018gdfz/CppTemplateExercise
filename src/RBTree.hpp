#ifndef RBTREE_HPP
#define RBTREE_HPP

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "Compare.hpp"
#include "common.h"

namespace mystd::rbtree {
enum NodeColor { RED, BLACK };

struct RBNodeBase {
  using BasePtr = RBNodeBase*;
  BasePtr left_, right_, parent_;
  NodeColor color_;
  RBNodeBase() {
    left_ = right_ = parent_ = nullptr;
    color_ = NodeColor::RED;
  }

  static auto isHeader(BasePtr node) -> bool {
    // 禁止对空树的 header 做判断
    assert(node->parent_ != nullptr);
    return node->color_ == NodeColor::RED && node->parent_->parent_ == node;
  }

  static auto minimum(BasePtr node) -> BasePtr {
    if (node == nullptr) {
      return nullptr;
    }
    while (node->left_ != nullptr) {
      node = node->left_;
    }
    return node;
  }
  static auto maximum(BasePtr node) -> BasePtr {
    if (node == nullptr) {
      return nullptr;
    }
    while (node->right_ != nullptr) {
      node = node->right_;
    }
    return node;
  }

  static auto prefix(BasePtr node) -> BasePtr {
    // prefix(header) := rightmost
    // prefix(leftmost) := header
    if (isHeader(node)) {
      return node->right_;
    }
    if (node->left_ != nullptr) {
      return maximum(node->left_);
    }
    BasePtr parent = node->parent_;
    while (node == parent->left_) {
      node = parent;
      parent = parent->parent_;
    }
    if (node->left_ != parent) {
      node = parent;
    }
    return node;
  }

  static auto suffix(BasePtr node) -> BasePtr {
    // suffix(header) := header
    // suffix(rightmost) := header
    if (isHeader(node)) {
      return node;
    }
    if (node->right_ != nullptr) {
      return minimum(node->right_);
    }
    BasePtr parent = node->parent_;
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

template <typename T>
struct RBNode : public RBNodeBase {
  using NodePtr = RBNode<T>*;

  T val_;

  template <typename... Args>
  explicit RBNode(Args&&... args)
      : RBNodeBase(), val_(std::forward<Args>(args)...) {}
};

// 节点存储的内容是 Value，可以通过 KeyOfValue 提取出比较键值，键值类型为 Key
template <typename Value, typename Key, typename KeyOfValue,
          typename Compare = mystd::compare::Less<Key>>
class RBTree {
public:
  using Node = RBNode<Value>;
  using NodePtr = Node*;
  using ConstNodePtr = const Node*;
  using BasePtr = RBNodeBase*;
  using ConstBasePtr = const RBNodeBase*;

  static const bool LEFT_SON = true;
  static const bool RIGHT_SON = false;

private:
  template <bool IsConst>
  class IteratorBase;

public:
  using Iterator = IteratorBase<false>;
  using ConstIterator = IteratorBase<true>;

  explicit RBTree(const Compare& comp = Compare()) : comp_(comp) {
    initHeader();
  }
  RBTree(const RBTree& other) : comp_(other.comp_) {
    initHeader();
    size_ = other.size_;
    if (other.root() != nullptr) {
      root() = copyTree(other.root(), header_);
      leftmost() = RBNodeBase::minimum(root());
      rightmost() = RBNodeBase::maximum(root());
    }
  }
  RBTree(RBTree&& other) noexcept
      : header_(other.header_),
        size_(other.size_),
        comp_(std::move(other.comp_)) {
    other.initHeader();
  }
  auto operator=(const RBTree& other) -> RBTree& {
    if (this != &other) {
      auto tmp(other);
      this->swap(tmp);
    }
    return *this;
  }
  auto operator=(RBTree&& other) noexcept -> RBTree& {
    if (this != &other) {
      clear();
      delete header_;
      header_ = other.header_;
      size_ = other.size_;
      comp_ = std::move(other.comp_);
      other.initHeader();
    }
    return *this;
  }

  ~RBTree() {
    clear();
    delete header_;
  }

  // clang-format off
  auto begin() noexcept -> Iterator { return Iterator(header_->left_); }
  auto begin() const noexcept -> ConstIterator { return ConstIterator(header_->left_); }
  auto end() noexcept -> Iterator { return Iterator(header_); }
  auto end() const noexcept -> ConstIterator { return ConstIterator(header_); }
  [[nodiscard]] auto size() const noexcept -> size_t { return size_; }
  [[nodiscard]] auto empty() const noexcept -> bool { return size_ == 0; }
  // clang-format on

  auto insert(const Value& val) -> std::pair<Iterator, bool> {
    auto [parent, whson] = findInsertPos(KeyOfValue()(val));
    auto new_node = new Node(val);
    return insertImpl(new_node, static_cast<NodePtr>(parent), whson);
  }
  auto insert(Value&& val) -> std::pair<Iterator, bool> {
    auto [parent, whson] = findInsertPos(KeyOfValue()(val));
    auto new_node = new Node(std::move(val));
    return insertImpl(new_node, static_cast<NodePtr>(parent), whson);
  }
  auto insertUnique(const Value& val) -> std::pair<Iterator, bool> {
    auto [parent, whson] = findInsertPos(KeyOfValue()(val));
    auto iter = Iterator(parent);
    if (parent != header_) {
      if (whson == LEFT_SON && parent != leftmost()) {
        --iter;
        if (!comp_(KeyOfValue()(*iter), KeyOfValue()(val))) {
          return {iter, false};
        }
      } else if (whson == RIGHT_SON) {
        if (!comp_(KeyOfValue()(*iter), KeyOfValue()(val))) {
          return {iter, false};
        }
      }
    }
    auto new_node = new Node(val);
    return insertImpl(new_node, static_cast<NodePtr>(parent), whson);
  }
  auto insertUnique(Value&& val) -> std::pair<Iterator, bool> {
    auto [parent, whson] = findInsertPos(KeyOfValue()(val));
    if (parent != header_) {
      // 这里 iter 一定能解引用
      auto iter = Iterator(parent);
      if (whson == LEFT_SON && parent != leftmost()) {
        --iter;
        if (!comp_(KeyOfValue()(*iter), KeyOfValue()(val))) {
          return {iter, false};
        }
      } else if (whson == RIGHT_SON) {
        if (!comp_(KeyOfValue()(*iter), KeyOfValue()(val))) {
          return {iter, false};
        }
      }
    }
    auto new_node = new Node(std::move(val));
    return insertImpl(new_node, static_cast<NodePtr>(parent), whson);
  }

  auto erase(Iterator iter) -> Iterator {
    if (iter == end()) {
      return end();
    }
    NodePtr node = static_cast<NodePtr>(iter.node_);
    bool has_two_children = (node->left_ != nullptr && node->right_ != nullptr);
    Iterator precomputed_next = iter;
    if (!has_two_children) {
      ++precomputed_next;
    }
    eraseImpl(node);
    if (has_two_children) {
      const Key new_key = KeyOfValue()(node->val_);
      if (!comp_(k, new_key) && !comp_(new_key, k)) {
        return Iterator(static_cast<BasePtr>(node));
      } else {
        return Iterator(RBNodeBase::suffix(static_cast<BasePtr>(node)));
      }
    } else {
      return precomputed_next;
    }
  }
  auto eraseUnique(const Key& key) -> size_t {
    auto iter = find(key);
    if (iter == end()) {
      return 0;
    }
    eraseImpl(static_cast<NodePtr>(iter.node_));
    return 1;
  }
  auto eraseMulti(const Key& key) -> size_t {
    auto iter = lowerBound(key);
    size_t count = 0;
    while (iter != end() && !comp_(key, KeyOfValue()(*iter)) &&
           !comp_(KeyOfValue()(*iter), key)) {
      // assert(iter != end());
      iter = erase(iter);
      count++;
    }
    return count;
  }

  void clear() {
    if (size_ > 0) {
      clearImpl();
      header_->parent_ = nullptr;
      header_->left_ = header_;
      header_->right_ = header_;
      size_ = 0;
    }
  }

private:
  BasePtr header_;
  size_t size_;
  Compare comp_;

  // clang-format off
  auto root() -> BasePtr& { return header_->parent_; }
  [[nodiscard]] auto root() const -> ConstBasePtr { return header_->parent_; }
  auto leftmost() -> BasePtr& { return header_->left_; }
  [[nodiscard]] auto leftmost() const -> ConstBasePtr { return header_->left_; }
  auto rightmost() -> BasePtr& { return header_->right_; }
  [[nodiscard]] auto rightmost() const -> ConstBasePtr { return header_->right_; }
  // clang-format on

  void initHeader() {
    header_ = new RBNodeBase();
    header_->color_ = NodeColor::RED;
    header_->parent_ = nullptr;
    header_->left_ = header_;
    header_->right_ = header_;
    size_ = 0;
  }

  void rotateLeft(BasePtr node) {
    BasePtr rson = node->right_;
    node->right_ = rson->left_;
    if (rson->left_ != nullptr) {
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
  void rotateRight(BasePtr node) {
    BasePtr lson = node->left_;
    node->left_ = lson->right_;
    if (lson->right_ != nullptr) {
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

  // 查找当前键应该插入的位置
  auto findInsertPos(const Key& key) -> std::pair<BasePtr, bool> {
    BasePtr node = root();
    BasePtr parent = header_;
    bool whson = LEFT_SON;

    while (node != nullptr) {
      parent = node;
      whson = comp_(key, KeyOfValue()(static_cast<NodePtr>(node)->val_));
      node = whson ? node->left_ : node->right_;
    }
    return {parent, whson};
  }
  void insertFix(NodePtr node) {
    while (node != root() && node->parent_->color_ == NodeColor::RED) {
      auto parent = static_cast<NodePtr>(node->parent_);
      auto g_parent = static_cast<NodePtr>(parent->parent_);
      if (parent == g_parent->left_) {
        auto uncle = static_cast<NodePtr>(g_parent->right_);
        if (uncle && uncle->color_ == NodeColor::RED) {
          uncle->color_ = parent->color_ = NodeColor::BLACK;
          g_parent->color_ = NodeColor::RED;
          node = g_parent;
        } else {
          if (node == parent->right_) {
            node = parent;
            rotateLeft(static_cast<BasePtr>(node));
            parent = static_cast<NodePtr>(node->parent_);
          }
          parent->color_ = NodeColor::BLACK;
          g_parent->color_ = NodeColor::RED;
          rotateRight(static_cast<BasePtr>(g_parent));
        }
      } else {
        auto uncle = static_cast<NodePtr>(g_parent->left_);
        if (uncle && uncle->color_ == NodeColor::RED) {
          uncle->color_ = parent->color_ = NodeColor::BLACK;
          g_parent->color_ = NodeColor::RED;
          node = g_parent;
        } else {
          if (node == parent->left_) {
            node = parent;
            rotateRight(static_cast<BasePtr>(node));
            parent = static_cast<NodePtr>(node->parent_);
          }
          parent->color_ = NodeColor::BLACK;
          g_parent->color_ = NodeColor::RED;
          rotateLeft(static_cast<BasePtr>(g_parent));
        }
      }
    }
    root()->color_ = NodeColor::BLACK;
  }
  auto insertImpl(NodePtr node, NodePtr parent, bool whson)
      -> std::pair<Iterator, bool> {
    node->parent_ = static_cast<BasePtr>(parent);
    if (parent == header_) {
      root() = static_cast<BasePtr>(node);
      leftmost() = rightmost() = static_cast<BasePtr>(node);
    } else if (whson == LEFT_SON) {
      parent->left_ = static_cast<BasePtr>(node);
      if (parent == leftmost()) {
        leftmost() = static_cast<BasePtr>(node);
      }
    } else {
      parent->right_ = static_cast<BasePtr>(node);
      if (parent == rightmost()) {
        rightmost() = static_cast<BasePtr>(node);
      }
    }
    insertFix(node);
    size_++;
    return {Iterator(static_cast<BasePtr>(node)), true};
  }

  void eraseFix(BasePtr node, BasePtr parent) {
    // 我们认为 node 现在额外多了一个黑色，目标是消除这个额外的黑色
    // 如果 node 本来是红色，那么直接改为黑色即可
    while (node != root() &&
           (node == nullptr || node->color_ == NodeColor::BLACK)) {
      if (node == parent->left_) {
        BasePtr bro = parent->right_;
        // case 1: 兄弟是红色
        if (bro->color_ == NodeColor::RED) {
          bro->color_ = NodeColor::BLACK;
          parent->color_ = NodeColor::RED;
          rotateLeft(parent);
          bro = parent->right_;
        }
        // 此处保证兄弟为黑色
        // case 2: 兄弟两儿子都是黑色
        if ((bro->left_ == nullptr || bro->left_->color_ == NodeColor::BLACK) &&
            (bro->right_ == nullptr ||
             bro->right_->color_ == NodeColor::BLACK)) {
          bro->color_ = NodeColor::RED;
          node = parent;
          parent = node->parent_;
        } else {
          // 此处保证兄弟至少存在一个红色的儿子
          // case 3: 兄弟左儿子红色，右儿子黑色
          if (bro->right_ == nullptr ||
              bro->right_->color_ == NodeColor::BLACK) {
            bro->left_->color_ = NodeColor::BLACK;
            bro->color_ = NodeColor::RED;
            rotateRight(bro);
            bro = parent->right_;
          }
          // 此处保证兄弟右儿子是红色
          bro->color_ = parent->color_;
          parent->color_ = NodeColor::BLACK;
          bro->right_->color_ = NodeColor::BLACK;
          rotateLeft(parent);
          break;
        }
      } else {
        BasePtr bro = parent->left_;
        if (bro->color_ == NodeColor::RED) {
          bro->color_ = NodeColor::BLACK;
          parent->color_ = NodeColor::RED;
          rotateRight(parent);
          bro = parent->left_;
        }
        if ((bro->left_ == nullptr || bro->left_->color_ == NodeColor::BLACK) &&
            (bro->right_ == nullptr ||
             bro->right_->color_ == NodeColor::BLACK)) {
          bro->color_ = NodeColor::RED;
          node = parent;
          parent = node->parent_;
        } else {
          if (bro->left_ == nullptr || bro->left_->color_ == NodeColor::BLACK) {
            bro->right_->color_ = NodeColor::BLACK;
            bro->color_ = NodeColor::RED;
            rotateLeft(bro);
            bro = parent->left_;
          }
          bro->color_ = parent->color_;
          parent->color_ = NodeColor::BLACK;
          bro->left_->color_ = NodeColor::BLACK;
          rotateRight(parent);
          break;
        }
      }
    }
    if (node != nullptr) {
      node->color_ = NodeColor::BLACK;
    }
  }
  void eraseImpl(NodePtr node) {
    NodePtr fin_erase = node;  // 最终删除的节点
    if (node->left_ && node->right_) {
      fin_erase =
          static_cast<NodePtr>(RBNodeBase::suffix(static_cast<BasePtr>(node)));
    }
    // 此时 fin_erase 只有一个孩子
    NodePtr son = fin_erase->left_ ? static_cast<NodePtr>(fin_erase->left_)
                                   : static_cast<NodePtr>(fin_erase->right_);

    if (son) {
      son->parent_ = fin_erase->parent_;
    }
    if (fin_erase == root()) {
      root() = static_cast<BasePtr>(son);
    } else if (fin_erase == fin_erase->parent_->left_) {
      fin_erase->parent_->left_ = static_cast<BasePtr>(son);
    } else {
      fin_erase->parent_->right_ = static_cast<BasePtr>(son);
    }

    if (fin_erase == leftmost()) {
      leftmost() = fin_erase->right_ ? RBNodeBase::minimum(fin_erase->right_)
                                     : fin_erase->parent_;
    }
    if (fin_erase == rightmost()) {
      rightmost() = fin_erase->left_ ? RBNodeBase::maximum(fin_erase->left_)
                                     : fin_erase->parent_;
    }
    if (fin_erase->color_ == NodeColor::BLACK) {
      eraseFix(static_cast<BasePtr>(son), fin_erase->parent_);
    }
    if (fin_erase != node) {
      node->val_ = std::move(fin_erase->val_);
    }
    delete fin_erase;
    size_--;
  }

  void clearImpl() {
    BasePtr node = root();
    while (node != nullptr) {
      if (node->left_ != nullptr) {
        node = node->left_;
      } else if (node->right_ != nullptr) {
        node = node->right_;
      } else {
        BasePtr parent = node->parent_;
        if (parent != nullptr && parent != header_) {
          if (parent->left_ == node) {
            parent->left_ = nullptr;
          } else {
            parent->right_ = nullptr;
          }
        }
        delete static_cast<NodePtr>(node);
        if (parent == header_) {
          node = nullptr;
        } else {
          node = parent;
        }
      }
    }
  }

  auto copyTree(ConstBasePtr src_root, BasePtr dst_parent) -> BasePtr {
    if (src_root == nullptr) {
      return nullptr;
    }
    auto node = new Node(static_cast<ConstNodePtr>(src_root)->val_);
    node->color_ = src_root->color_;
    node->parent_ = dst_parent;
    node->left_ = copyTree(src_root->left_, static_cast<BasePtr>(node));
    node->right_ = copyTree(src_root->right_, static_cast<BasePtr>(node));
    return static_cast<BasePtr>(node);
  }

  template <bool IsConst>
  class IteratorBase {
    friend class RBTree;
    template <bool>
    friend class IteratorBase;

    using Reference = std::conditional_t<IsConst, const Value&, Value&>;
    using Pointer = std::conditional_t<IsConst, const Value*, Value*>;
    using NodePointer = std::conditional_t<IsConst, const Node*, Node*>;

  private:
    BasePtr node_;

  public:
    explicit IteratorBase(BasePtr node = nullptr) : node_(node) {}

    // 允许 non-const 向 const 赋值
    template <bool B, std::enable_if_t<IsConst && !B, int> = 0>
    IteratorBase(const IteratorBase<B>& other) : node_(other.node_) {}

    auto operator*() const -> Reference {
      return static_cast<NodePointer>(node_)->val_;
    }
    auto operator->() const -> Pointer {
      return &(static_cast<NodePointer>(node_)->val_);
    }

    // 前置++
    auto operator++() -> IteratorBase& {
      node_ = RBNodeBase::suffix(node_);
      return *this;
    }
    // 后置++
    auto operator++(int) -> IteratorBase {
      IteratorBase temp(node_);
      node_ = RBNodeBase::suffix(node_);
      return temp;
    }
    // 前置--
    auto operator--() -> IteratorBase& {
      node_ = RBNodeBase::prefix(node_);
      return *this;
    }
    // 后置--
    auto operator--(int) -> IteratorBase {
      IteratorBase temp(node_);
      node_ = RBNodeBase::prefix(node_);
      return temp;
    }

    template <bool B>
    auto operator==(const IteratorBase<B>& other) const -> bool {
      return node_ == other.node_;
    }
    template <bool B>
    auto operator!=(const IteratorBase<B>& other) const -> bool {
      return node_ != other.node_;
    }

    /// WARNING: 仅在 Set.hpp 和 Map.hpp 中使用
    [[nodiscard]] auto base() const -> BasePtr { return node_; }
  };

public:
  auto find(const Key& key) const -> ConstIterator {
    ConstBasePtr node = root();
    ConstBasePtr ans = header_;
    while (node != nullptr) {
      Key knode = KeyOfValue()(static_cast<ConstNodePtr>(node)->val_);
      if (comp_(key, knode)) {
        node = node->left_;
      } else {
        ans = node;
        node = node->right_;
      }
    }
    if (ans == header_ ||
        comp_(KeyOfValue()(static_cast<ConstNodePtr>(ans)->val_), key)) {
      return end();
    }
    return ConstIterator(const_cast<BasePtr>(ans));
  }
  auto find(const Key& key) -> Iterator {
    ConstIterator citer = static_cast<const RBTree*>(this)->find(key);
    return Iterator(citer.node_);
  }

  auto lowerBound(const Key& key) const -> ConstIterator {
    ConstBasePtr node = root();
    ConstBasePtr ans = header_;
    while (node != nullptr) {
      Key knode = KeyOfValue()(static_cast<ConstNodePtr>(node)->val_);
      if (!comp_(knode, key)) {
        ans = node;
        node = node->left_;
      } else {
        node = node->right_;
      }
    }
    return ConstIterator(const_cast<BasePtr>(ans));
  }
  auto lowerBound(const Key& key) -> Iterator {
    ConstIterator citer = static_cast<const RBTree*>(this)->lowerBound(key);
    return Iterator(citer.node_);
  }
  auto upperBound(const Key& key) const -> ConstIterator {
    ConstBasePtr node = root();
    ConstBasePtr ans = header_;
    while (node != nullptr) {
      Key knode = KeyOfValue()(static_cast<ConstNodePtr>(node)->val_);
      if (comp_(key, knode)) {
        ans = node;
        node = node->left_;
      } else {
        node = node->right_;
      }
    }
    return ConstIterator(const_cast<BasePtr>(ans));
  }
  auto upperBound(const Key& key) -> Iterator {
    ConstIterator citer = static_cast<const RBTree*>(this)->upperBound(key);
    return Iterator(citer.node_);
  }

  auto equalRange(const Key& key) const
      -> std::pair<ConstIterator, ConstIterator> {
    return {lowerBound(key), upperBound(key)};
  }
  auto equalRange(const Key& key) -> std::pair<Iterator, Iterator> {
    auto [clower, cupper] = static_cast<const RBTree*>(this)->equalRange(key);
    return {Iterator(clower.node_), Iterator(cupper.node_)};
  }

  void swap(RBTree& other) noexcept {
    mystd::swap(header_, other.header_);
    mystd::swap(size_, other.size_);
    mystd::swap(comp_, other.comp_);
  }
};
}  // namespace mystd::rbtree

#endif