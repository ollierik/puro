#include <array>
#include <vector>
#include <memory>


template <class ElementType, int PoolSize>
class StackMemory
{
public:
    constexpr size_t size() { return PoolSize; };
    ElementType& operator[] (int i)
    {
        return (ElementType&)elements[i * sizeof(ElementType)];
    }

    ElementType* ptr(int i)
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
            , lookupIndex(i)
        {}

        ElementType& operator*() const
        {
            return pool.elements[lookupIndex];
        }

        bool operator!= (const Iterator& other)
        {
            //if (depleted() && other.depleted())
            return ! depleted();
            // TODO other cases
            // Now can't do iterator comparison, is this even needed?
        }

        Iterator& operator++()
        {
           lookupIndex++;
           return *this;
        }

    private:

        bool depleted() const
        {
            return (lookupIndex >= pool.used);
        }

        Pool<ElementType, PoolSize>& pool;
        int lookupIndex;

    };

    Pool()
    {
        for (int i=0; i<PoolSize; ++i)
        {
            lookup[i] = i;
        }
    }

    Pool::Iterator begin()
    {
        return Pool::Iterator(*this, 0);
    }

    /** Essentially a dummy value, since comparison is not done with this */
    Pool::Iterator end()
    {
        return Pool::Iterator(*this, PoolSize);
    }

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

    void remove(Pool<ElementType, PoolSize>::Iterator* it)
    {
        *it.print();
        // TODO out of range check
    }

    constexpr size_t size()
    {
        return elements.size();
    }

    size_t used = 0;
    StackMemory<ElementType, PoolSize> elements;
    StackMemory<size_t, PoolSize> lookup;
};
