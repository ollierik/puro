#pragma once

#include <atomic>

template <typename ElementType>
struct Node
{
    Node() : next(nullptr) {}

    /** Not guaranteed to be constructed */
    ElementType* getElement()
    {
        //return reinterpret_cast<ElementType*> (element);
        return &element;
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
    //void* element[sizeof(ElementType)];
    ElementType element;
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

    /** Non-threadsafe */
    static void insert(Node<ElementType>* node, Node<ElementType>* insertedAfter)
    {
        node->next = insertedAfter->next;
        insertedAfter->next = node;
    }

    /** Non-threadsafe */
    static void remove(Node<ElementType>* node, Node<ElementType>* prev)
    {
        prev->next = node->next;
        node->next = nullptr;
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
    Iterator(SafePool<ElementType>& sp, Node<ElementType>* n)
        : pool(sp)
        , node(n)
        , prev(nullptr)
    {
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

        // if no actives, try to get an addition
        if (node == nullptr)
        {
            auto* n = pool.added.pop_front();
            pool.active.push_front(n);
            node = n;
            prev = nullptr;
        }
        // if we're at the end of list, start going through the additions
        // move additions to the end of the active list
        else if (node->next == nullptr)
        {
            if (! pool.added.empty())
            {
                auto* n = pool.added.pop_front();
                List<ElementType>::insert(n, node);
            }

            prev = node;
            node = node->next;
        }
        else
        {
            prev = node;
            node = node->next;
        }


        return *this;
    }

    Node<ElementType>* popCurrent()
    {
        // if first
        if (prev == nullptr)
        {
            auto* popped = pool.active.pop_front();
            node = pool.active.first();
            return popped;
        }
        else
        {
            auto* popped = node;

            List<ElementType>::remove(node, prev);

            node = prev;

            return popped;
        }
    }

    void printIter()
    {
        std::cout << "prev, current, next:" << std::endl;;
        if (prev != nullptr) prev->print();
        else std::cout << "nullptr" << std::endl;

        if (node) node->print();
        else std::cout << "nullptr" << std::endl;

        if (node && node->next != nullptr) node->next->print();
        else std::cout << "nullptr" << std::endl;

        std::cout << "####################" << std::endl;;
    }

    Node<ElementType>* node;
    Node<ElementType>* prev; // only the iterator is doubly linked

private:

    SafePool<ElementType>& pool;
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
            auto it = Iterator<ElementType> (*this, active.first());
            ++it;
            return it;
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
    }

private:

    friend class Iterator<ElementType>;

    List<ElementType> active;
    List<ElementType> added;
    List<ElementType> inactive;

    size_t numElements;
};