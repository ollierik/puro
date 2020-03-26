#include <vector>
#include <memory>

/** Stores elements into stack, fixed size. */
template <class ElementType, int PoolSize>
class StackMemoryManager
{
public:
    constexpr size_t size() { return PoolSize; };
    ElementType& operator[] (size_t i)
    {
        return reinterpret_cast<ElementType&>(elements[i * sizeof(ElementType)]);
    }

    ElementType* ptr(size_t i)
    {
        return reinterpret_cast<ElementType*>(&elements[i * sizeof(ElementType)]);
    }
private:
    void* elements[PoolSize * sizeof(ElementType)];
};



/** Pool with fixed amount of elements. Allocates elements to stack.
*/
template <class ElementType, int PoolSize>
class StackPool
{
public:

    class Iterator
    {
    public:

        Iterator(StackPool& p, int i)
            : pool(p)
            , iteratorIndex(i)
        {}

        Iterator& operator*()
        {
            return *this;
        }

        ElementType* operator->()
        {
            return getElement();
        }

        bool operator!= (const int& /* dummy */)
        {
            return shouldContinue();
        }

        Iterator& operator++()
        {
            ++iteratorIndex;
            return *this;
        }
        
        void print()
        {
            std::cout << "Iterator: " << std::endl;
            std::cout << "    iteratorIndex: " << iteratorIndex << std::endl;
            std::cout << "    lookupIndex: " << getLookupIndex() << std::endl;
        }

    private:

        friend class StackPool<ElementType, PoolSize>;

        const size_t getLookupIndex() { return pool.lookup[iteratorIndex]; }
        ElementType* getElement() { return &pool.elements[getLookupIndex()]; }

        bool shouldContinue() const { return (iteratorIndex < pool.numInUse); }

        StackPool<ElementType, PoolSize>& pool;
        int iteratorIndex;
    };

    StackPool()
    {
        for (int i=0; i<PoolSize; ++i)
        {
            lookup[i] = i;
        }
    }

    StackPool::Iterator begin() { return StackPool::Iterator(*this, 0); }

    /** Returns int, used as a flag to check for depletion in the Iterator's ++ */
    int end() { return 0; } 

    template <class ...Args>
    ElementType* add(Args... args)
    {
        if (numInUse != PoolSize)
        {
            ElementType* mem = elements.ptr(numInUse++);
            ElementType* e = new (mem) ElementType(args...);
            return e;
        }
        return nullptr;
    }

    void remove(StackPool::Iterator& it)
    {
        std::cout << "\nREMOVE: " << it.getLookupIndex() << std::endl << std::endl;;

        const auto indexToRemove = it.getLookupIndex();
        const auto indexOfLast = --numInUse;

        if (indexOfLast > 0)
        {
            const auto temp = lookup[indexToRemove];
            lookup[indexToRemove] = lookup[indexOfLast];
            lookup[indexOfLast] = temp;
        }

        it.iteratorIndex -= 1;
    }

    constexpr size_t size()
    {
        return elements.size();
    }

    size_t numInUse = 0;
    StackMemoryManager<ElementType, PoolSize> elements;
    StackMemoryManager<size_t, PoolSize> lookup;
};

template <class ElementType, int InitialPoolSize = 32, int MaxPoolSize = -1>
class DynamicPool
{
public:

    using ElementPtr = std::unique_ptr<ElementType>;

    class Iterator
    {
    public:

        Iterator(StackPool& p, int i)
            : pool(p)
            , iteratorIndex(i)
        {}

        Iterator& operator*()
        {
            return *this;
        }

        ElementType* operator->()
        {
            return getElement();
        }

        bool operator!= (const int& /* dummy */)
        {
            return shouldContinue();
        }

        Iterator& operator++()
        {
            ++iteratorIndex;
            return *this;
        }
        
        void print()
        {
            std::cout << "Iterator: " << std::endl;
            std::cout << "    iteratorIndex: " << iteratorIndex << std::endl;
            std::cout << "    lookupIndex: " << getLookupIndex() << std::endl;
        }

    private:

        //friend class StackPool<ElementType, PoolSize>;

        const size_t getLookupIndex() { return pool.lookup[iteratorIndex]; }
        ElementType* getElement() { return &pool.elements[getLookupIndex()]; }

        bool shouldContinue() const { return (iteratorIndex < pool.numInUse); }

        //StackPool<ElementType, PoolSize>& pool;
        int iteratorIndex;
    };


    DynamicPool()
    {
        for (int i=0; i<PoolSize; ++i)
        {
            lookup[i] = i;
        }
    }

    DynamicPool::Iterator begin() { return DynamicPool::Iterator(*this, 0); }

    /** Returns int, used as a flag to check for depletion in the Iterator's ++ */
    int end() { return 0; } 

    template <class ...Args>
    ElementType* add(Args... args)
    {
        // if we have already allocated elements that are not in use
        if (numInUse < elements.size())
        {
        
        }
        // if we need to create new unique_ptrs
        else if (elements.size() == elements.capacity())
        {
            if (! attemptToIncreaseSize())
                return nullptr;

        }

        elements.emplace_back(std::forward(std::make_unique<ElementType>(args...));
    }

    void remove(DynamicPool::Iterator& it)
    {
        std::cout << "\nREMOVE: " << it.getLookupIndex() << std::endl << std::endl;;

        const auto indexToRemove = it.getLookupIndex();
        const auto indexOfLast = --numInUse;

        if (indexOfLast > 0)
        {
            const auto temp = lookup[indexToRemove];
            lookup[indexToRemove] = lookup[indexOfLast];
            lookup[indexOfLast] = temp;
        }

        it.iteratorIndex -= 1;
    }

    size_t size() const
    {
        return elements.size();
    }

    bool tryToIncreaseCapacity()
    {
        const auto capacityBefore = elements.capacity();
        int targetCapacity = elements.capacity() * 2;
        if (MaxPoolSize < 0 && targetCapacity > MaxPoolSize)
            targetCapacity = MaxPoolSize;

        elements.reserve(targetCapacity);

        return (elements.capacity > capacityBefore);
    }

    size_t numInUse = 0;
    std::vector<ElementType> elements;
};
