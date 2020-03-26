#include <memory>
#include <forward_list>
#include <vector>

/** Pool with fixed amount of elements. Allocates elements to stack.
*/
template <class ElementType, int PoolSize>
class FixedPool
{
public:

    /** Contains elements and allocates memory for them, fixed capacity. */
    template <class ElementType, int Capacity>
    class Container
    {
    public:
        constexpr size_t size() { return Capacity; };
        ElementType& operator[] (size_t i)
        {
            return reinterpret_cast<ElementType&>(elements[i * sizeof(ElementType)]);
        }

        ElementType* ptr(size_t i)
        {
            return reinterpret_cast<ElementType*>(&elements[i * sizeof(ElementType)]);
        }
    private:
        void* elements[Capacity * sizeof(ElementType)];
    };


    class Iterator
    {
    public:

        Iterator(FixedPool& p)
            : pool(p)
            , iteratorIndex(0)
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

        friend class FixedPool<ElementType, PoolSize>;

        const size_t getLookupIndex() { return pool.lookup[iteratorIndex]; }
        ElementType* getElement() { return &pool.elements[getLookupIndex()]; }

        bool shouldContinue() const { return (iteratorIndex < pool.numInUse); }

        FixedPool<ElementType, PoolSize>& pool;
        int iteratorIndex;
    };

    FixedPool()
    {
        for (int i=0; i<PoolSize; ++i)
        {
            lookup[i] = i;
        }
    }

    FixedPool::Iterator begin() { return FixedPool::Iterator(*this, 0); }

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

    void remove(FixedPool::Iterator& it)
    {
        /*
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
        */
        remove(it.getLookupIndex());
        --it.iteratorIndex;
    }

    void remove(size_t lookupIndex)
    {
        std::cout << "\nREMOVE: " << lookupIndex() << std::endl << std::endl;;
        const auto indexOfLast = --numInUse;

        if (indexOfLast > 0)
        {
            const auto temp = lookup[indexToRemove];
            lookup[indexToRemove] = lookup[indexOfLast];
            lookup[indexOfLast] = temp;
        }
    }


    constexpr size_t size()
    {
        return elements.size();
    }

 private:

    size_t numInUse = 0;
    Container<ElementType, PoolSize> elements;
    Container<size_t, PoolSize> lookup;
};




template <class ElementType, int ChunkSize = 32>
class DynamicPool
{
public:

    template <class ElementType, int PoolSize>
    class PoolList
    {
    public:

        PoolList() : next(nullptr) {}

        PoolList* allocateNext()
        {
            next = std::make_unique<PoolList> ();
            return next.get();
        }

        FixedPool<ElementType, PoolSize> pool;
        std::unique_ptr<PoolList> next;
    };

    class Iterator
    {
    public:
        Iterator(DynamicPool& p)
            : dynamicPool(p)
            , poolList(dynamicPool.poolList.get())
        {
        }

        Iterator& operator++()
        {
            if (iteratorIndex >= )
            return *this;
        }

    private:

        int iteratorIndex = 0;
        DynamicPool& dynamicPool;
        PoolList* poolList;
    };

    DynamicPool()
    {
    }

    template <class ...Args>
    ElementType* add(Args... args)
    {
        // if pool list hasn't been initialised, create it and add to it
        if (poolList == nullptr)
        {
            poolList = std::make_unique<PoolList> ();
            return poolList->pool.add(args...);
        }

        // try to add to existing containers
        // iterate through pool list to access subpools
        PoolList* pl = poolList.get();
        while (pl != nullptr)
        {
            ElementType* e = pl->pool.add(args...);
            if (e != nullptr)
            {
                return e;
            }
            pl = pl.getNext();
        }
        // all existing containers full, create new and add to it
        // TODO check for size here to ensure we are allowed to do this
        pl = poolList->allocateNext();
        return pl->pool.add(args...);
    }

    void remove(DynamicPool::Iterator& it)
    {
    }

    //std::forward_list<FixedPool<ElementType, ChunkSize>> containers;
    std::unique_ptr<PoolList<ElementType, ChunkSize>> poolList;
};
