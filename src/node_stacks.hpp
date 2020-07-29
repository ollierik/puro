#pragma once

namespace puro {

/** Singly-linked node that is contained in a Stack */
template <typename T>
class Node
{
public:
    Node() : next(nullptr) {}

    /** Not guaranteed to be constructed */
    T& getElement()
    {
        return *reinterpret_cast<T*> (element);
    }

    Node* next;

private:

    // workaround to skip the need for a default ctor
    void* element[sizeof(T)];
};


/** Non-threadsafe stack which contains Nodes. More flexibility than with the thread-safe version. */
template <typename T>
class NodeStack
{
public:

    /** Iterator to access NodeStack contents with range-based loop */
    class Iterator
    {
    public:
        Iterator(NodeStack& s, Node<T>* n)
            : stack(s)
            , prev(nullptr)
            , node(n)
            , next((node != nullptr ? node->next : nullptr))
        {
        }

        T& get() { return node->getElement(); }

        bool operator!= (const Iterator& other) { return node != other.node; }
        Iterator& operator*() { return *this; }
        Iterator& operator++()
        {
            if (node == nullptr)
                return *this;

            prev = node;
            node = next;
            next = (node == nullptr ? nullptr : node->next);

            return *this;
        }

        Node<T>* prev; // only the iterator is doubly linked
        Node<T>* node;
        Node<T>* next;

        NodeStack& stack;
    };

    ///=========================================================

    NodeStack() : head(nullptr) {}

    /** Check if the list is empty */
    bool empty() const { return head == nullptr; }

    /** Get the first element of the list */
    Node<T>* first() { return head; }

    /** Push to the front of the list */
    void push_front(Node<T>* node)
    {
        if (node == nullptr)
            return;

        node->next = head;
        head = node;
    }

    /** Push a forward-linked set of Nodes into the stack */
    void push_multiple(Node<T>* node)
    {
        if (node == nullptr)
            return;

        // traverse to find the last element
        Node<T>* tail = node;
        while (tail->next != nullptr)
            tail = tail->next;

        tail->next = head;
        head = node;
    }

    /** Pop from the front of the list */
    Node<T>* pop_front()
    {
        if (head == nullptr)
            return nullptr;

        Node<T>* popped = head;
        head = head->next;
        popped->next = nullptr;

        return popped;
    }

    Node<T>* pop(Iterator& it)
    {
        Node<T>* popped = it.node;

        // if iterator was at end
        if (popped == nullptr)
        {
            return nullptr;
        }
        // if it is the first element
        else if (popped == head)
        {
            head = head->next;
            it.node = head;
            it.next = it.node;
            popped->next = nullptr;
            return popped;
        }

        // if element is not the first one
        it.prev->next = popped->next;
        it.next = popped->next;
        it.node = it.prev;
        popped->next = nullptr;

        return popped;
    }

    /** Pop all of the elements from the stack.
        The returned element points to the first element of the stack, and should be pushed to another stack with push_multiple() */
    Node<T>* pop_all()
    {
        Node<T>* node = head;
        head = nullptr;

        return node;
    }

    Iterator begin() { return Iterator(*this, head); }
    Iterator end() { return Iterator(*this, nullptr); }

    Node<T>* head;
};




/** Atomic stack containing Nodes. All operations should go through push_front and pop_front.
    This container can't be iterated on. To iterate, the contents should be first transferred to
    an instance of NodeStack, for example via pop_all(). */
template <typename T>
class SafeStack
{
public:
    SafeStack() : head(nullptr)
    {
    }

    /** Check if the list is empty */
    bool empty() const { return head.load() == nullptr; }

    /** Get the first element of the list */
    Node<T>* first() { return head.load(); }

    /** Atomically push to the front of the list */
    void push_front(Node<T>* node)
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
        {
        }; // empty body
    }

    /** Atomically push a forward-linked set of Nodes into the stack */
    void push_multiple(Node<T>* node)
    {
        if (node == nullptr)
            return;

        // traverse to find the last element
        Node<T>* tail = node;
        while (tail->next != nullptr)
            tail = tail->next;

        // put the current value of head into the tail->next
        tail->next = head.load(std::memory_order_relaxed);

        while (!head.compare_exchange_weak(tail->next, node,
            std::memory_order_release,
            std::memory_order_relaxed))
            ; // empty body
    }

    /** Atomically pop from the front of the stack */
    Node<T>* pop_front()
    {
        Node<T>* node = head.load();

        if (node == nullptr)
            return nullptr;

        while (!head.compare_exchange_weak(node, node->next,
            std::memory_order_release,
            std::memory_order_relaxed))
            ; // empty body

        if (node != nullptr)
            node->next = nullptr;

        return node;
    }

    /** Atomically pop all of the elements from the stack.
        The returned element points to the first element of the stack, and should be pushed to another stack with push_multiple() */
    Node<T>* pop_all()
    {
        Node<T>* node = head.load(std::memory_order_relaxed);

        while (!head.compare_exchange_weak(node, nullptr,
            std::memory_order_release,
            std::memory_order_relaxed))
            ; // empty body

        return node;
    }

    std::atomic<Node<T>*> head;
};


/** A helper class to own the memory the Stacks use. */
template <typename T>
class ChunkMemoryAllocator
{
    struct Chunk
    {
        Chunk(int size)
        {
            memory.resize(size);
        }

        std::vector<Node<T>> memory;
        std::unique_ptr<Chunk> next;
    };

public:
    ChunkMemoryAllocator() : head(nullptr) {}

    template <typename StackType>
    void allocateChunk(int numElements, StackType& pushToStack)
    {
        Chunk* newChunk = new Chunk(numElements);
        newChunk->next.reset(head.release());

        head.reset(newChunk);

        for (Node<T>& node : newChunk->memory)
        {
            pushToStack.push_front(&node);
        }
    }

private:
    std::unique_ptr<Chunk> head;
};

}
