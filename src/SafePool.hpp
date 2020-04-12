#pragma once

#include <atomic>

template <typename ElementType>
struct Node
{
    Node(ElementType e) : element(e), next(nullptr) {}

    ElementType element;
    Node* next;
};
 
template <typename ElementType>
class List
{
 public:

    void push_front(Node<ElementType>* node)
    {
        if (node == nullptr)
            return;

        // put the current value of head into the new node->next
        node->next = head.load(std::memory_order_relaxed);
 
        // try to replace head with new node
        // if head != node, i.e., if some other thread got here first:
        // load head into node->next and try again
        while (! head.compare_exchange_weak(node->next, node,
                                              std::memory_order_release,
                                              std::memory_order_relaxed))
            ; // empty body
    }

    Node<ElementType>* pop_front()
    {
        Node<ElementType>* node = head.load();

        if (node == nullptr)
            return nullptr;

        while (! head.compare_exchange_weak(node, node->next(),
                                          std::memory_order_release,
                                          std::memory_order_relaxed))
            ; // empty body

        return node;
    }

    Node<ElementType>* first()
    {
        return head.load();
    }

private:
    std::atomic<Node<ElementType>*> head;
};

template <class ElementType>
class SafePool;

template <class ElementType>
class Iterator
{
public:
    Iterator(SafePool<ElementType>& sp, Node<ElementType>* n)
        : pool(sp)
        , node(n)
        , prev(nullptr)
        , skipIncrement(false)
        , processingAdditions(false)
    {}

    bool operator!= (const Iterator<ElementType>& other)
    {
        return node != other.node;
    }

    Iterator& operator++()
    {
        // processing actives
        if (! processingAdditions)
        {
            if (skipIncrement) // flag is set if current element was removed
            {
                skipIncrement = false;
                return *this;
            }

            // active list empty, continue to additions
            if (node->next == nullptr)
            {
                processingAdditions = true;
            }
            else
            {
                prev = node;
                node = node->next;
            }
            return *this;
        }
        // processing additions
        else
        {
            // remove from added-list, add to active list
            node = pool.added.pop_front();

            // if not added depleted
            if (node != nullptr)
                pool.active.push_front(*node);

            return *this;
        }
    }

    void popCurrent()
    {
        if (prev != nullptr)
        {
            prev->next = node->next;
            return node;
        }
        else
        {
            pool.active.pop_front();
        }
    }

    Node<ElementType>* node;
    Node<ElementType>* prev; // only the iterator is doubly linked

private:
    SafePool<ElementType>& pool;
    bool skipIncrement;
    bool processingAdditions;
};



template <class ElementType>
class SafePool
{
public:

    using ElementPtr = ElementType*;

    SafePool()
    {
    }

    void reserve(size_t n)
    {
        errorif(numElements != 0, "reserve called twice, resizing not yet implemented");
        numElements = n;
        elements = operator new (sizeof(ElementType) * n);

        for (auto i=0; i<numElements; ++i)
        {
            Node<ElementPtr>* node = new Node<ElementPtr> (getRawElementWithIndex(i));
            inactive.push_front(node);
        }
    }

    Iterator<ElementType> begin()
    {
        return Iterator<ElementType> (*this, active.front());
    }

    Iterator<ElementType> end()
    {
        return Iterator<ElementType> (*this, nullptr);
    }

    template <typename... Args>
    ElementType* add(Args... args)
    {
        Node<ElementType>* node = inactive.pop_front();

        if (node != nullptr)
        {
            // call ctor before adding to list
            auto* e = new (&node->element) ElementType(args...);
            added.push_front(node);
            return e;
        }
        return nullptr;
    }

    void remove(Iterator<ElementType>& it)
    {
        auto* n = it.popCurrent();
        errorif(n == nullptr, "shouldn't try to remove nullptr node");
        inactive.push_front(*n);
    }

private:

    ElementType* getRawElementWithIndex(size_t i)
    {
        //return reinterpret_cast<ElementType*> (&elements[i * sizeof(ElementType)]);
        return (reinterpret_cast<ElementType*> (elements) + i);
    }

    void* elements = nullptr;
    size_t numElements;

    List<ElementPtr> active;
    List<ElementPtr> added;
    List<ElementPtr> inactive;
};