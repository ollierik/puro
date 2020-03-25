#include <array>
#include <vector>
#include <memory>


template <class ElementType, int PoolSize>
class StackMemory
{
public:
    constexpr size_t size() { return PoolSize; };
    ElementType& operator[] (size_t i)
    {
        return (ElementType&)elements[i * sizeof(ElementType)];
    }

    ElementType* ptr(size_t i)
    {
        //return (ElementType*)(&elements[i * sizeof(ElementType)]);
        return reinterpret_cast<ElementType*>(&elements[i * sizeof(ElementType)]);
    }
private:
    void* elements[PoolSize * sizeof(ElementType)];
};



/** Pool with fixed amount of elements. Allocates elements to stack.
*/
template <class ElementType, int PoolSize>
class Pool
{
public:

    class Iterator
    {
    public:

        Iterator(Pool& p, int i)
            : pool(p)
            , iteratorIndex(i)
        {}

        //ElementType& operator*() const
        Iterator& operator*()
        {
            return *this;
        }

        ElementType* operator->()
        {
            const auto lookupIndex = getLookupIndex();
            return &pool.elements[lookupIndex];
        }

        bool operator!= (const int& end)
        {
            return ! depleted();
        }

        Iterator& operator++()
        {
            ++iteratorIndex;
            return *this;
        }

    private:

        friend class Pool<ElementType, PoolSize>;

        const size_t getLookupIndex() { return pool.lookup[iteratorIndex]; }

        bool depleted() const { return (iteratorIndex >= pool.used); }

        Pool<ElementType, PoolSize>& pool;
        int iteratorIndex;
    };

    Pool()
    {
        for (int i=0; i<PoolSize; ++i)
        {
            lookup[i] = i;
        }
    }

    Pool::Iterator begin() { return Pool::Iterator(*this, 0); }

    /** Returns int, used as a flag to check for depletion in the Iterators ++ */
    int end() { return 0; } 

    template <class ...Args>
    ElementType* add(Args... args)
    {
        if (used != PoolSize)
        {
            ElementType* mem = elements.ptr(used++);
            ElementType* e = new (mem) ElementType(args...);
            return e;
        }
        return nullptr;
    }

    void remove(Pool::Iterator& it)
    {
        std::cout << "removing: " << it.getLookupIndex() << std::endl;

        const auto indexToRemove = it.getLookupIndex();
        const auto indexOfLast = --used;

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

    size_t used = 0;
    StackMemory<ElementType, PoolSize> elements;
    StackMemory<size_t, PoolSize> lookup;
};
