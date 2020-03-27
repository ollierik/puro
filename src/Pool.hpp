#include <memory>
#include <forward_list>
#include <vector>
#include <cassert>

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
        ElementType* getElement() { return pool.getElement(iteratorIndex); }

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

    FixedPool::Iterator begin() { return FixedPool::Iterator(*this); }

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
        remove(it.getLookupIndex());
        --it.iteratorIndex;
    }

    void remove(size_t lookupIndex)
    {
        std::cout << "\nREMOVE: " << lookupIndex << std::endl << std::endl;;

        const auto indexToRemove = lookupIndex;
        const auto indexOfLast = --numInUse;

        if (indexOfLast > 0)
        {
            const auto temp = lookup[indexToRemove];
            lookup[indexToRemove] = lookup[indexOfLast];
            lookup[indexOfLast] = temp;
        }
    }

    ElementType* getElement(int index)
    {
        const auto lookupIndex = lookup[index];
        return &elements[lookupIndex];
    }

    constexpr size_t capacity()
    {
        return PoolSize;
    }

    size_t size() const
    {
        return numInUse;
    }

 protected:

    size_t numInUse = 0;
    Container<ElementType, PoolSize> elements;
    Container<size_t, PoolSize> lookup;
};


template <class ElementType, int PoolSize = 32>
class DynamicPool
{
public:

    template<class ElementType, int PoolSize>
    class Iterator;

    template <class ElementType, int PoolSize>
    class SubPool : public FixedPool<ElementType, PoolSize>
    {
    public:

        SubPool() : nextPool(nullptr) {}

        SubPool<ElementType, PoolSize>* allocateNext()
        {
            nextPool = std::make_unique<SubPool<ElementType, PoolSize>> ();
            return nextPool.get();
        }

        SubPool<ElementType, PoolSize>* next() { return nextPool.get(); }

    private:

        std::unique_ptr<SubPool> nextPool;
    };

    template <class ElementType, int PoolSize>
    class Iterator
    {
    public:
        Iterator(DynamicPool<ElementType, PoolSize>& p)
            : dynamicPool(p), subPool(dynamicPool.subPools.get())
        {
        }

        Iterator& operator++()
        {
            const auto s = subPool->size();
            if (iteratorIndex >= s)
            {
                subPool = subPool->next();
                iteratorIndex = 0;
            }
            ++iteratorIndex;
            return *this;
        }

        Iterator& operator*()
        {
            return *this;
        }

        ElementType* operator->()
        {
            return getElement();
        }

        bool operator!= (const int& /* dummy */) const
        {
            if (subPool == nullptr)
                return false;

             return (iteratorIndex < subPool->size());
        }

        void print()
        {
            std::cout << "Iterator: " << std::endl;
            std::cout << "    iteratorIndex: " << iteratorIndex << std::endl;
            //std::cout << "    lookupIndex: " << getLookupIndex() << std::endl;
        }

    private:

        friend class DynamicPool<ElementType, PoolSize>;

        ElementType* getElement()
        {
            assert(subPool != nullptr);
            //return subPool->getElement(subPool->lookup[iteratorIndex]);
            return subPool->getElement(iteratorIndex);
        }

        /*
        int getLookupIndex()
        {
            assert((subPool != nullptr));
            return subPool->lookup[iteratorIndex];
        }
        */

        int iteratorIndex;
        DynamicPool& dynamicPool;
        SubPool<ElementType, PoolSize>* subPool;
    };


    DynamicPool()
    {
    }

    DynamicPool::Iterator<ElementType, PoolSize> begin() { return DynamicPool::Iterator<ElementType, PoolSize>(*this); }

    /** Returns int, used as a flag to check for depletion in the Iterator's ++ */
    int end() { return 0; } 

    template <class ...Args>
    ElementType* add(Args... args)
    {
        // if pool list hasn't been initialised, create it and add to it
        if (subPools == nullptr)
        {
            subPools = std::make_unique<SubPool<ElementType, PoolSize>> ();
            return subPools->add(args...);
        }

        // try to add to existing containers
        // iterate through pool list to access subpools
        SubPool<ElementType, PoolSize>* sp = subPools.get();
        while (sp != nullptr)
        {
            ElementType* e = sp->add(args...);
            if (e != nullptr)
            {
                return e;
            }
            sp = sp->next();
        }
        // all existing containers full, create new and add to it
        // TODO check for size here to ensure we are allowed to do this
        sp = sp->allocateNext();
        return sp->add(args...);
    }

    void remove(DynamicPool::Iterator<ElementType, PoolSize>& it)
    {
        it.subPool->remove(it.getLookupIndex());
        it.iteratorIndex--;
    }

    size_t size()
    {
        size_t sum = 0;
        SubPool<ElementType, PoolSize>* sp = subPools.get();
        while (sp != nullptr)
        {
            sum += sp->size();
            sp = sp->next();
        }
        return sum;
    }

    size_t capacity()
    {
        size_t sum = 0;
        SubPool<ElementType, PoolSize>* sp = subPools.get();
        while (sp != nullptr)
        {
            sum += sp->capacity();
            sp = sp->next();
        }
        return sum;
    }

    std::unique_ptr<SubPool<ElementType, PoolSize>> subPools;
};
