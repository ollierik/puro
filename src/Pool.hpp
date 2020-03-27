#include <memory>
#include <cassert>
#include <cstddef>

/** Contains elements and allocates memory for them, fixed capacity.
    Elements are guaranteed not to move after constructing them.
    The order of access changes as elements are added and removed.
*/

template <class ElementType, int Capacity>
class PoolMemory
{
public:

    using ElementPtr = ElementType*;

    PoolMemory()
    {
        std::cout << "Initialise mapping:\n";
        for (int i=0; i<Capacity; i++)
        {
            ElementPtr e = getElementAtMemoryLocation(i);
            mapping[i] = e;
            std::cout << i << ": " << e << std::endl;

            accessToMapping[i] = i;
        }
    }

    /** Return number of active elements in the container */
    int size() { return numInUse; };

    /** Return the total capacity of the container */
    constexpr int capacity() { return Capacity; };

    /** Calls getElementWithAccessIndex() */
    ElementType& operator[] (int accessIndex)
    {
        return *getElementWithAccessIndex(accessIndex);
    }

    /** Returns the ElementType* with direct query. The Element is not guaranteed to be allocated or active */
    ElementType* getElementAtMemoryLocation(int i)
    {
        return reinterpret_cast<ElementType*> (&elements[ i * sizeof(ElementType) ]);
    }

    /** Returns ElementType& with access index. The access order is not guaranteed, and should be only used with iteration.
        The Element is not guaranteed to be allocated, active, or the same as something previously allocated. */
    ElementType* getElementWithAccessIndex(int accessIndex)
    {
        ElementType* e = mapping[accessIndex];
        return e;
    }

    template <class ...Args>
    ElementType* add(Args... args)
    {
        if (numInUse != Capacity)
        {
            // get first free position
            ElementType* mem = mapping[numInUse++];

            std::cout << "Add to: " << mem << std::endl;

            // construct in-place
            ElementType* e = new (mem) ElementType(args...);
            return e;
        }
        return nullptr;
    }

    void release(ElementType* e)
    {
        // TODO bounds check

        ElementPtr* mappingPos = elementToMapping(e);
        ElementPtr* lastPos = &mapping[--numInUse];

        std::cout << "mappingPos points to index: " << offsetInMemory(*mappingPos) << std::endl;
        std::cout << "lastPos points to index: " << offsetInMemory(*lastPos) << std::endl;
 
        // swap positions of released element and last element in the mapping
        {
            ElementPtr tempVal = *mappingPos;
            *mappingPos = *lastPos;
            *lastPos = tempVal;
        }

        const int atmIndex1 = offsetInMemory(*mappingPos);
        const int atmIndex2 = offsetInMemory(*lastPos);

        // swap inverse lookups
        //swap<int>(&accessToMapping[atmIndex1], &accessToMapping[atmIndex2]);
        {
            int tempVal = accessToMapping[atmIndex1];
            accessToMapping[atmIndex1] = accessToMapping[atmIndex2];
            accessToMapping[atmIndex2] = tempVal;
        }
    }

private:

    ElementPtr* elementToMapping(ElementType* e)
    {
        //std::cout << "elementToMapping:" << std::endl;
        const int offset = offsetInMemory(e);
        //std::cout << "offset in memory: " << offset << std::endl;
        const int accessIndex = accessToMapping[offset];
        return &mapping[accessIndex];
    }

    int offsetInMemory(ElementType* e)
    {
        int offset = (e - getElementAtMemoryLocation(0)) / sizeof(ElementType*);
        return offset;
    }

    int numInUse = 0;

    // Can't use array, since ElementType is not expected to have default constructor
    void* elements[Capacity * sizeof(ElementType)];

    ElementPtr mapping[Capacity];
    int accessToMapping[Capacity];
};


/** Pool with fixed amount of elements. Allocates elements to stack.
*/

template <class ElementType, int PoolSize>
class FixedPool
{
public:

    template <class ElementType, int PoolSize>
    class Iterator
    {
    public:

        Iterator(PoolMemory<ElementType, PoolSize>& e, int i)
            : elements(e)
            , accessIndex(i)
        {}

        Iterator& operator*()
        {
            return *this;
        }

        ElementType* operator->()
        {
            return elements[accessIndex];
        }

        bool operator!= (const FixedPool<ElementType, PoolSize>& other)
        {
            return accessIndex != other.accessIndex;
        }

        Iterator& operator++()
        {
            --accessIndex;
            return *this;
        }
        
        void print()
        {
            std::cout << "Iterator: " << std::endl;
            std::cout << "    accessIndex: " << accessIndex << std::endl;
        }

    private:

        friend class FixedPool<ElementType, PoolSize>;

        PoolMemory<ElementType, PoolSize>& elements;
        int accessIndex;
    };


    /////////////////////////////////////////////////////////////////////////

    FixedPool()
    {
    }

    FixedPool::Iterator<ElementType, PoolSize> begin() { return FixedPool::Iterator(elements, elements.size()-1); }
    FixedPool::Iterator<ElementType, PoolSize> end() { return FixedPool::Iterator(elements, -1); }

    template <class ...Args>
    ElementType* add(Args... args)
    {
        return elements.add(args...);
    }

    void remove(ElementType* element)
    {
        elements.release(element);
    }

    constexpr int capacity() const
    {
        return PoolSize;
    }

    int size() const
    {
        return elements.size();
    }

 protected:

    PoolMemory<ElementType, PoolSize> elements;
};
