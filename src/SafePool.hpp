#pragma once

#include <atomic>

/*
TODO
- separate lists and iterators for active and added
    - active works in typical manner, also has prev node if need to remove
    - added works by popping from added queue until empty
*/

template <typename ElementType>
struct Node
{
    Node(ElementType e) : element(e), next(nullptr), prev(nullptr) {}

    ElementType element;
    Node* next;
};
 
template<typename ElementType>
class List
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

/*
template <class ElementType>
class Iterator
{
    Iterator(Node<ElementType>* n) : node(n) {}

    bool operator!= (const Iterator<ElementType>& other)
    {
        return node != other.node;
    }

    Node<ElementType>* node;
};
*/

template <class ElementType>
class SafePool
{
public:

    using ElementPtr = ElementType*;

    SafePool()
    {
        // TODO, do some other way
        reserve(16);
    }

    void reserve(size_t n)
    {
        errorif(numElements != 0, "reserve called twice, resizing not yet implemented");
        numElements = n;
        elements = operator new (sizeof(ElementType * numElements));

        for (auto i=0; i<numElements; ++i)
        {
            Node<ElementPtr> node = new Node<ElementPtr> (getRawElementWithIndex(i));
            inactive.push(node);
        }
    }

    template <typename... Args>
    ElementType* add(Args... args)
    {
        Node<ElementType>* node = inactive.pop();

        if (node != nullptr)
        {
            // create first, add to list second
            new (&node->element) ElementType(args);
            added.push(node);
            return &node->element;
        }
        return nullptr;
    }

    void remove(Node<ElementPtr>& node)
    {

    }

    void setActive(Iterator& it)
    {
        // remove from current list (known by the iterator), push to active
    }

private:

    ElementType* getRawElementWithIndex(size_t i)
    {
        return reinterpret_cast<ElementType*> (elements[i * sizeof(ElementType)]);
    }

    void* elements = nullptr;
    size_t numElements;

    List<ElementPtr> active;
    List<ElementPtr> added;
    List<ElementPtr> inactive;
};