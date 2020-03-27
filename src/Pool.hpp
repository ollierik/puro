#pragma once 

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
    int size() const { return numInUse; };

    /** Return the total capacity of the container */
    constexpr int capacity() const { return Capacity; };

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
 
        // swap positions of released element and last element in the mapping
        {
            ElementPtr tempVal = *mappingPos;
            *mappingPos = *lastPos;
            *lastPos = tempVal;
        }

        // swap inverse lookups
        {
            const int atmIndex1 = offsetInMemory(*mappingPos);
            const int atmIndex2 = offsetInMemory(*lastPos);

            int tempVal = accessToMapping[atmIndex1];
            accessToMapping[atmIndex1] = accessToMapping[atmIndex2];
            accessToMapping[atmIndex2] = tempVal;
        }
    }

    bool containedHere(ElementType* element)
    {
        ElementType* e0 = (getElementAtMemoryLocation(0));
        ElementType* en = (getElementAtMemoryLocation(Capacity));

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

    // Can't use array, since ElementType is not expected to have a default constructor
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

    FixedPool() = default;

    /////////////////////////////////////////////////////////////////////////

    template <class ElementType, int PoolSize>
    class Iterator
    {
    public:

        Iterator(PoolMemory<ElementType, PoolSize>& e, int i)
            : elements(e)
            , accessIndex(i)
        {}

        ElementType* operator*()
        {
            return &elements[accessIndex];
        }

        ElementType* operator->()
        {
            return elements[accessIndex];
        }

        bool operator!= (const Iterator<ElementType, PoolSize>& other)
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

        PoolMemory<ElementType, PoolSize>& elements;
        int accessIndex;
    };

    FixedPool::Iterator<ElementType, PoolSize> begin()
    {
        return FixedPool<ElementType, PoolSize>::Iterator<ElementType, PoolSize>(elements, elements.size()-1);
    }
    FixedPool::Iterator<ElementType, PoolSize> end()
    {
        return FixedPool<ElementType, PoolSize>::Iterator<ElementType, PoolSize>(elements, -1);
    }

    /////////////////////////////////////////////////////////////////////////


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

    int size()
    {
        return elements.size();
    }

 protected:

    PoolMemory<ElementType, PoolSize> elements;
};
