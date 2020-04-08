#pragma once

#include <atomic>

template <typename ElementType>
struct Node
{
    Node(const ElementType& e) : element(e), next(nullptr) {}

    ElementType element;
    Node* next;
};
 
template<typename ElementType>
class Stack 
{
 public:

    void push(Node<ElementType>& node)
    {
        // put the current value of head into new_node->next
        node->next = head.load(std::memory_order_relaxed);
 
        // try to replace head with new node
        // if head != node, i.e. some other thread got here first
        // load head into node->next and try again
        while (! head.compare_exchange_weak(node->next, node,
                                              std::memory_order_release,
                                              std::memory_order_relaxed))
            ; // empty body
    }

    Node<ElementType>* pop()
    {
        Node<ElementType>* node = head.load();

        if (node == nullptr)
            return nullptr;

        while (! head.compare_exchange_weak(node, node-next(),
                                          std::memory_order_release,
                                          std::memory_order_relaxed))
            ; // empty body

        return node;
    }

private:
    std::atomic<Node<ElementType>*> head;
};

template <class ElementType>
class SafePool
{
public:

    ElementType* allocate()
    {
        Node<ElementType>* node = inactive.pop();
        if (n != nullptr)
        {
        
        }
        return &node->element;
    }

    std::atomic<List<ElementType>> active;
    std::atomic<List<ElementType>> added;
    std::atomic<List<ElementType>> inactive;
};