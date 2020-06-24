#pragma once

#include <atomic>


/** A partially thread-safe pool object that contains and owns objects.
    Adding elements via add() function is thread-safe. Removing should only be performed by the consumer thread. */
template <class ElementType>
class SafePool
{
    /** Singly-linked node that is contained in a List */
    template <typename ElementType>
    class Node
    {
    public:
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

    /** List containing Nodes, has an atomic head */
    template <typename ElementType>
    class List
    {
    public:
        List() : head(nullptr)
        {
        }
        /** Check if the list is empty */
        bool empty() const { return head.load() == nullptr; }

        /** Get the first element of the list */
        Node<ElementType>* first() { return head.load(); }

        /** Non-threadsafe, doesn't do any nullptr checks */
        static void insert(Node<ElementType>* node, Node<ElementType>* insertedAfter)
        {
            node->next = insertedAfter->next;
            insertedAfter->next = node;
        }

        /** Non-threadsafe, doesn't do any nullptr checks */
        static void remove(Node<ElementType>* node, Node<ElementType>* prev)
        {
            prev->next = node->next;
            node->next = nullptr;
        }

        /** Atomically push to the front of the list */
        void push_front(Node<ElementType>* node)
        {
            if (node == nullptr)
                return;

            // put the current value of head into the new node->next
            node->next = head.load(std::memory_order_relaxed);

            // try to replace head with new node
            // if head != node, i.e., if some other thread got here first:
            // load head into node->next and try again
            while (!head.compare_exchange_weak(node->next, node,
                std::memory_order_release,
                std::memory_order_relaxed))
                ; // empty body
        }

        /** Atomically pop from the front of the list */
        Node<ElementType>* pop_front()
        {
            Node<ElementType>* node = head.load();

            if (node == nullptr)
                return nullptr;

            while (!head.compare_exchange_weak(node, node->next,
                std::memory_order_release,
                std::memory_order_relaxed))
                ; // empty body

            return node;
        }

    private:
        std::atomic<Node<ElementType>*> head;
    };

    /** Iterator to access pool contents with range-based loop */
    template <class ElementType>
    class Iterator
    {
    public:
        Iterator(SafePool<ElementType>& sp, Node<ElementType>* n, bool startFromAdditions = false)
            : pool(sp)
            , node(n)
            , prev(nullptr)
        {
            if (startFromAdditions)
            {
                auto* popped = pool.added.pop_front();
                pool.active.push_front(popped);
                node = popped;
                prev = nullptr;
            }
        }

        Iterator (const Iterator& other)
            : pool(other.pool)
            , node(other.node)
            , prev(other.prev)
        {
            /** You are most likely using while (auto ...)
                This will screw up the iteration.
                Use while (auto& ...) instead.
                The copy constructor should be omited as per C++ RVO specs. */
            errorif(true, "Direct access to iterator copy constructor");
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
            if (node == nullptr)
                return *this;

            // if we're at the end of list, start going through the additions
            // move additions to the end of the active list
            if (node->next == nullptr)
            {
                if (!pool.added.empty())
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

    /** Actual SafePool template class implementation */
public:

    SafePool()
        : numAllocated(0)
        , numElements(0)
    {
    }

    /** Increases the size of the container, if n > capacity() */
    void reserve(size_t n)
    {
        size_t currentCapacity = numAllocated.load();
        if (n < currentCapacity)
            return;

        numAllocated.compare_exchange_strong(currentCapacity, n,
            std::memory_order_release,
            std::memory_order_relaxed);

        auto delta = n - currentCapacity;

        for (auto i = 0; i < delta; ++i)
        {
            Node<ElementType>* node = new Node<ElementType> ();
            inactive.push_front(node);
        }
    }

    Iterator<ElementType> begin()
    {
        if (active.empty())
        {
            return Iterator<ElementType> (*this, active.first(), true);
        }

        return Iterator<ElementType> (*this, active.first());
    }

    Iterator<ElementType> end()
    {
        return Iterator<ElementType> (*this, nullptr);
    }

    size_t capacity() const
    {
        return numAllocated.load();
    }

    size_t size() const
    {
        return numElements.load();
    }

    template <typename... Args>
    ElementType* add(Args... args)
    {
        Node<ElementType>* node = inactive.pop_front();

        if (node != nullptr)
        {
            // call ctor before adding to list
            ElementType* e = new (node->getElement()) ElementType(args...);
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

    std::atomic<size_t> numAllocated;
    std::atomic<size_t> numElements;
};