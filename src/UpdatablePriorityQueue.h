#pragma once

#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <stack>
#include <utility>

#include "Util.h"  

template <typename T, typename CompT = std::less<>>
struct UpdatablePriorityQueue : private CompT
{
    // TODO: constexpr log
    static constexpr float alpha = 0.75f;
    // beta = 1/(-log2(alpha))
    // static constexpr float beta = 1.0f / -std::log2(alpha);
    static constexpr float beta = 2.4094208396532;

    struct Node
    {
        Node* parent;
        Node* left;
        Node* right;
        T value;

        Node(const T& value) :
            parent(nullptr),
            left(nullptr),
            right(nullptr),
            value(value)
        {

        }

        Node(T&& value) :
            parent(nullptr),
            left(nullptr),
            right(nullptr),
            value(std::move(value))
        {

        }
    };

    using UninitializedNode = std::aligned_storage_t<sizeof(Node), alignof(Node)>;

    using NodeHandle = Node*;

    UpdatablePriorityQueue(int capacity, CompT cmp = {}) :
        CompT(std::move(cmp)),
        m_capacity(capacity),
        m_size(0),
        m_maxSize(0),
        m_nextNode(0),
        m_root(nullptr),
        m_values(std::make_unique<UninitializedNode[]>(capacity))
    {
        m_rebuildTreeTemporaryNodeStorage.reserve(capacity);
    }

    UpdatablePriorityQueue(UpdatablePriorityQueue&& other) noexcept :
        CompT(other),
        m_capacity(other.m_capacity),
        m_size(other.m_size),
        m_maxSize(other.m_maxSize),
        m_nextNode(other.m_nextNode),
        m_root(other.m_root),
        m_values(std::move(other.m_values)),
        m_rebuildTreeTemporaryNodeStorage(std::move(other.m_rebuildTreeTemporaryNodeStorage))
    {
        other.m_root = nullptr;
    }

    UpdatablePriorityQueue& operator=(UpdatablePriorityQueue&& other) noexcept
    {
        cleanup(m_root);

        CompT::operator=(other);

        m_capacity = other.m_capacity;
        m_size = other.m_size;
        m_maxSize = other.m_maxSize;
        m_nextNode = other.m_nextNode;
        m_values = std::move(other.m_values);
        m_root = other.m_root;

        other.m_root = nullptr;

        return *this;
    }

    template <typename FuncT>
    void forEach(FuncT&& f)
    {
        forEach(m_root, std::forward<FuncT>(f));
    }

    template <typename FuncT>
    void forEach(Node* node, FuncT&& f)
    {
        if (node != nullptr)
        {
            f(node);
            forEach(node->left, std::forward<FuncT>(f));
            forEach(node->right, std::forward<FuncT>(f));
        }
    }

    [[nodiscard]] const T& top()
    {
        return minNode()->value;
    }

    void pop()
    {
        erase(minNode());
    }

    template <typename FuncT>
    void update(Node* node, FuncT&& f)
    {
        assert(node != nullptr);

        f(node->value);
        if (!isWellPlaced(node))
        {
            eraseNoDestroy(node);
            insert(node);
        }
    }

    void erase(Node* node) 
    {
        assert(node != nullptr);

        eraseNoDestroy(node);
        destroyNode(node);
    }

    template <typename... ArgsTs>
    [[nodiscard]] Node* emplace(ArgsTs&& ... args)
    {
        assert(m_nextNode < m_capacity);
        Node* node = createNode(m_nextNode++, std::forward<ArgsTs>(args)...);
        insert(node);
        return node;
    }

    template <typename U>
    [[nodiscard]] Node* push(U&& value)
    {
        assert(m_nextNode < m_capacity);
        Node* node = createNode(m_nextNode++, std::forward<U>(value));
        insert(node);
        return node;
    }

    [[nodiscard]] bool empty() const
    {
        return m_size == 0;
    }

    [[nodiscard]] int size() const
    {
        return m_size;
    }

    ~UpdatablePriorityQueue()
    {
        cleanup(m_root);
    }

private:
    int m_capacity;
    int m_size;
    int m_maxSize;
    int m_nextNode;
    Node* m_root;
    std::unique_ptr<UninitializedNode[]> m_values;
    std::vector<Node*> m_rebuildTreeTemporaryNodeStorage;

    template <typename... ArgsTs>
    [[nodiscard]] Node* createNode(int i, ArgsTs&& ... args)
    {
        Node* ptr = reinterpret_cast<Node*>(&(m_values[i]));
        new (ptr) Node(std::forward<ArgsTs>(args)...);
        return ptr;
    }

    void destroyNode(Node* node)
    {
        node->~Node();
    }

    void cleanup(Node* node)
    {
        if (node != nullptr)
        {
            cleanup(node->left);
            cleanup(node->right);
            destroyNode(node);
        }
    }

    [[nodiscard]] bool compare(const Node& lhs, const Node& rhs) const noexcept
    {
        // duplicates are hard to handle normally so we include pointers in comparision
        // as a tie breaker (since pointers are stable)
        return CompT::operator()(lhs.value, rhs.value) || (!CompT::operator()(rhs.value, lhs.value) && std::less<>()(&lhs, &rhs));
    }

    void eraseNoDestroy(Node* node) 
    {
        eraseBst(node);
        node->parent = nullptr;
        node->left = nullptr;
        node->right = nullptr;

        // if we delete the last element we don't want to rebuild
        if (m_root != nullptr && m_size <= alpha * m_maxSize) {
            rebuildTree(m_root);
        }
    }

    [[nodiscard]] bool isWellPlaced(Node* node)
    {
        if (node->left != nullptr && !compare(*(node->left), *node))
        {
            return false;
        }

        if (node->right != nullptr && !compare(*node, *(node->right)))
        {
            return false;
        }

        if (node->parent != nullptr)
        {
            if (node->parent->left == node && !compare(*node, *(node->parent)))
            {
                return false;
            }
            else if(!compare(*(node->parent), *node))
            {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] Node* minNode(Node* node)
    {
        while (node->left != nullptr)
        {
            node = node->left;
        }

        return node;
    }

    [[nodiscard]] Node* minNode()
    {
        return minNode(m_root);
    }

    [[nodiscard]] int subtreeSize(const Node* node) const
    {
        if (node == nullptr)
        {
            return 0;
        }

        return 1 + subtreeSize(node->left) + subtreeSize(node->right);
    }

    [[nodiscard]] Node* sibling(Node* node)
    {
        if (node->parent != nullptr) 
        {
            if (node->parent->left == node) 
            {
                return node->parent->right;
            }
            else 
            {
                return node->parent->left;
            }
        }

        return nullptr;
    }

    [[nodiscard]] Node* findScapegoat(Node* node)
    {
        int height = 0;
        int totalSize = 1;
        while (node->parent != nullptr) 
        {
            height += 1;
            totalSize += 1 + subtreeSize(sibling(node));

            if (height > rebuildThreshold(totalSize))
            {
                return node->parent;
            }

            node = node->parent;
        }

        return nullptr;
    }
    
    Node* transplant(Node* nodeToReplace, Node* newNode) 
    {
        if (nodeToReplace->parent == nullptr) 
        {
            m_root = newNode;
        }
        else if (nodeToReplace == nodeToReplace->parent->left) 
        {
            nodeToReplace->parent->left = newNode;
        }
        else 
        {
            nodeToReplace->parent->right = newNode;
        }

        if (newNode != nullptr) 
        {
            newNode->parent = nodeToReplace->parent;
        }

        return newNode;
    }
    
    void insert(Node* node)
    {
        const int height = insertBst(node);
        if (height > rebuildThreshold(m_size)) 
        {
            rebuildTree(findScapegoat(node));
        }
    }

    // This functions stores inorder traversal 
    // of tree rooted with ptr in an array arr[] 
    void storeInArray(Node* node)
    {
        if (node == nullptr)
        {
            return;
        }

        storeInArray(node->left);
        m_rebuildTreeTemporaryNodeStorage.emplace_back(node);
        storeInArray(node->right);
    }

    void rebuildTree(Node* scapegoat) 
    {
        m_rebuildTreeTemporaryNodeStorage.clear();

        Node* p = scapegoat->parent;
        storeInArray(scapegoat);
        Node* rebuiltTreeRoot = buildTree(std::begin(m_rebuildTreeTemporaryNodeStorage), std::end(m_rebuildTreeTemporaryNodeStorage));
        if (p == nullptr)
        {
            m_root = rebuiltTreeRoot;
            m_root->parent = nullptr;
        }
        else if (p->right == scapegoat)
        {
            p->right = rebuiltTreeRoot;
            p->right->parent = p;
        }
        else
        {
            p->left = rebuiltTreeRoot;
            p->left->parent = p;
        }

        m_maxSize = m_size;
    }

    [[nodiscard]] Node* buildTree(typename std::vector<Node*>::iterator start, typename std::vector<Node*>::iterator end)
    {
        if (start == end) {
            return nullptr;
        }

        const auto middle = start + std::distance(start, end) / 2; // div ceil

        // middle becomes root of subtree instead of scapegoat
        Node* node = *middle;

        // recursively get left and right nodes
        node->left = buildTree(start, middle);
        if (node->left != nullptr)
        {
            node->left->parent = node;
        }

        node->right = buildTree(middle + 1, end);
        if (node->right != nullptr)
        {
            node->right->parent = node;
        }

        return node;
    }

    Node* eraseBst(Node* deleteNode)
    {
        Node* replacementNode = nullptr;

        if (deleteNode != nullptr)
        {
            if (deleteNode->left == nullptr)
            {
                replacementNode = transplant(deleteNode, deleteNode->right);
            }
            else if (deleteNode->right == nullptr)
            {
                replacementNode = transplant(deleteNode, deleteNode->left);
            }
            else
            {
                Node* successorNode = minNode(deleteNode->right);
                if (successorNode->parent != deleteNode)
                {
                    transplant(successorNode, successorNode->right);

                    successorNode->right = deleteNode->right;
                    successorNode->right->parent = successorNode;
                }

                transplant(deleteNode, successorNode);

                successorNode->left = deleteNode->left;
                successorNode->left->parent = successorNode;

                replacementNode = successorNode;
            }

            m_size -= 1;
        }

        return replacementNode;
    }

    // returns depth of insertion
    int insertBst(Node* node)
    {
        Node* next = m_root;
        if (next == nullptr)
        {
            m_root = node;
            m_size += 1;
            return 0;
        }

        int height = 1;
        for(;;)
        {
            if (compare(*node, *next))
            {
                if (next->left == nullptr)
                {
                    next->left = node;
                    node->parent = next;
                    break;
                }
                else
                {
                    next = next->left;
                }
            }
            else // we can assume all values to be distinct
            {
                if (next->right == nullptr)
                {
                    next->right = node;
                    node->parent = next;
                    break;
                }
                else
                {
                    next = next->right;
                }
            }

            height += 1;
        }

        m_size += 1;

        return height;
    }

    [[nodiscard]] int rebuildThreshold(int size) 
    {
        return static_cast<int>(std::log2(size) * beta);
    }
};
