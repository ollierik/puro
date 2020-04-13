#pragma once

#include <atomic>

template <typename ElementType>
struct Node
{
    Node() : next(nullptr) {}

    /** Not guaranteed to be constructed */
    ElementType* getElement()
    {
        return reinterpret_cast<ElementType*> (element);
    }

    void print()
    {
        std::cout << "this: ";
        getElement()->print();
        std::cout << "\tnext: ";
        if (next == nullptr)
            std::cout << "nullptr\n";
        else
            next->getElement()->print();
    }

    Node* next;
private:

    // workaround to skip the need for a default ctor
    void* element[sizeof(ElementType)];
};
 
template <typename ElementType>
class List
{
 public:

     List() : head(nullptr)
     {
     }

     bool empty() const
     {
         return head.load() == nullptr;
     }

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

        while (! head.compare_exchange_weak(node, node->next,
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
    Iterator(SafePool<ElementType>& sp, Node<ElementType>* n, bool startFromAdditions = false)
        : pool(sp)
        , node(n)
        , prev(nullptr)
        , processingAdditions(startFromAdditions)
    {
        if (processingAdditions)
        {
            popAndGetNextAdded();
        }
    }

    bool operator!= (const Iterator<ElementType>& other)
    {
        return node != other.node;
    }

    Iterator<ElementType>& operator*()
    {
        return *this;
    }

    ElementType* operator->()
    {
        errorif(node == nullptr, "trying to access nullptr element");
        return node->getElement();
    }

    Iterator<ElementType>& operator++()
    {
        //printIter();
        // processing actives
        if (processingAdditions)
        {
            popAndGetNextAdded();
        }
        getNextActive();
        return *this;
    }

    Node<ElementType>* popCurrent()
    {
        //std::cout << "POP CURRENT\n";
        // if not first
        if (prev != nullptr)
        {
            auto* popped = node;

            prev->next = node->next;

            std::cout << "REMOVING: ";
            popped->print();

            node = prev;

            return popped;
        }
        else
        {
            auto* n = pool.active.pop_front();
            node = pool.active.first();
            return n;
        }
    }

    void printIter()
    {
        std::cout << "prev, current, next:" << std::endl;;
        if (prev != nullptr)
        prev->print();
        node->print();
        if (node->next != nullptr)
            node->next->print();

        std::cout << "####################" << std::endl;;
    }

    Node<ElementType>* node;
    Node<ElementType>* prev; // only the iterator is doubly linked

private:


    void getNextActive()
    {
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
    }

    void popAndGetNextAdded()
    {
        // remove from added-list, add to active list
        node = pool.added.pop_front();
        prev = nullptr;

        pool.active.push_front(node);
    }

    SafePool<ElementType>& pool;
    bool processingAdditions;
};



template <class ElementType>
class SafePool
{
public:

    SafePool() : numElements(0)
    {
    }

    void reserve(size_t n)
    {
        errorif(numElements != 0, "reserve called twice, resizing not yet implemented");
        numElements = n;

        for (auto i=0; i<numElements; ++i)
        {
            Node<ElementType>* node = new Node<ElementType> ();
            inactive.push_front(node);
        }
    }

    Iterator<ElementType> begin()
    {
        if (active.empty())
        {
            return Iterator<ElementType> (*this, added.first(), true);
        }

        return Iterator<ElementType> (*this, active.first());
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
            auto* e = new (node->getElement()) ElementType(args...);
            added.push_front(node);
            return e;
        }
        return nullptr;
    }

    void remove(Iterator<ElementType>& it)
    {
        auto* n = it.popCurrent();
        errorif(n == nullptr, "shouldn't try to remove nullptr node");
        inactive.push_front(n);
        //it.printIter();
    }

private:

    friend class Iterator<ElementType>;

    int currentActiveIndex = 0;
    List<ElementType>* currentActive;
    List<ElementType> active[2];
    List<ElementType> added;
    List<ElementType> inactive;

    size_t numElements;
};