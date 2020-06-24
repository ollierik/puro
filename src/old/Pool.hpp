#pragma once 


// TODO
// - Since FixedPool never seems to outperform DynamicPool, refactor and loose FixedPool

template <class ElementType, int Capacity>
class PoolMemory
{
public:

    using ElementPtr = ElementType*;

    PoolMemory()
    {
        for (int i=0; i<Capacity; i++)
        {
            ElementPtr e = getElementAtMemoryLocation(i);
            mapping[i] = e;
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

    ElementType* allocate()
    {
        const auto cap = Capacity;
        if (numInUse < cap)
        {
            // get first free position
            ElementType* mem = mapping[numInUse++];
            return mem;
        }
        return nullptr;
    }

    void release(int accessIndex)
    {
        // TODO bounds check

        ElementPtr* mappingPos = &mapping[accessIndex];
        ElementPtr* lastPos = &mapping[--numInUse];
 
        // swap positions of released element and last element in the mapping
        ElementPtr tempVal = *mappingPos;
        *mappingPos = *lastPos;
        *lastPos = tempVal;
    }

private:

    int numInUse = 0;

    // Can't use array, since ElementType is not expected to have a default constructor
    void* elements[Capacity * sizeof(ElementType)];
    ElementPtr mapping[Capacity];
};



/** Pool with fixed amount of elements. Allocates elements to stack.
*/

template <class ElementType, int PoolSize>
class FixedPool
{
public:

    /** Contains elements and allocates memory for them, fixed capacity.
        Elements are guaranteed not to move after constructing them.
        The order of access changes as elements are added and removed.
    */


    FixedPool() = default;

    /////////////////////////////////////////////////////////////////////////


    /** Template names prefixed with N because of MSVC */
    template <class NElementType, int NPoolSize>
    class Iterator
    {
    public:

        Iterator(PoolMemory<NElementType, PoolSize>& e, int i)
            : elements(e)
            , accessIndex(i)
        {}

        Iterator<NElementType, PoolSize>& operator*()
        {
            return *this;
        }

        NElementType* operator->()
        {
            return &elements[accessIndex];
        }

        NElementType* getElement()
        {
            return elements->getElementWithAccessIndex(accessIndex);
        }

        bool operator!= (const Iterator<NElementType, PoolSize>& other)
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

        friend class FixedPool<NElementType, PoolSize>;

        PoolMemory<NElementType, PoolSize>& elements;
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


    ElementType* allocate()
    {
        return elements.allocate();
    }

    void remove(Iterator<ElementType, PoolSize>& iterator)
    {
        elements.release(iterator.accessIndex);
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



template <class ElementType, int ChunkAllocationSize>
class PoolChunk : public PoolMemory<ElementType, ChunkAllocationSize>
{
public:

    PoolChunk() = default;

    PoolChunk* allocateNext()
    {
        std::cout << "*** Allocate new PoolChunk" << std::endl;
        nextChunk = std::make_unique<PoolChunk<ElementType, ChunkAllocationSize>> ();
        return nextChunk.get();
    }

    PoolChunk<ElementType, ChunkAllocationSize>* next() { return nextChunk.get(); }

private:
    std::unique_ptr<PoolChunk<ElementType, ChunkAllocationSize>> nextChunk = nullptr;
};


template <class ElementType, int ChunkAllocationSize = 32>
class DynamicPool
{
public:

    DynamicPool() = default;

    template <class NElementType, int NChunkAllocationSize = 32>
    class Iterator
    {
    public:
        Iterator(PoolChunk<NElementType, NChunkAllocationSize>* c)
            : chunk(c)
            , chunkAccessIndex(chunk == nullptr ? -1 : chunk->size()-1)
        {
        }

        Iterator<NElementType, NChunkAllocationSize>& operator*()
        {
            return *this;
        }

        NElementType* operator->()
        {
            if (chunk != nullptr)
            {
                return chunk->getElementWithAccessIndex(chunkAccessIndex);
            }
            return nullptr;
        }

        Iterator& operator++()
        {
            if (--chunkAccessIndex < 0)
            {
                chunk = chunk->next();
                chunkAccessIndex = (chunk == nullptr ? -1 : chunk->size() -1);
            }
            return *this;
        }

        bool operator!= (const Iterator<NElementType, NChunkAllocationSize>& other)
        {
            return ! (chunkAccessIndex == other.chunkAccessIndex && chunk == other.chunk);
        }

    private:

        friend class DynamicPool<NElementType, NChunkAllocationSize>;

        PoolChunk<NElementType, NChunkAllocationSize>* chunk;
        int chunkAccessIndex;
    };

    Iterator<ElementType, ChunkAllocationSize> begin()
    {
        return Iterator<ElementType, ChunkAllocationSize> (chunks.get());
    };
    Iterator<ElementType, ChunkAllocationSize> end()
    {
        return Iterator<ElementType, ChunkAllocationSize> (nullptr);
    };

    ElementType* allocate()
    {
        // TODO implement size constraint
        
        // if pool list hasn't been initialised, create it and add to it
        if (chunks == nullptr)
        {
            chunks = std::make_unique<PoolChunk<ElementType, ChunkAllocationSize>> ();

            combinedCapacity += ChunkAllocationSize;
            ++numInUse;

            return chunks->allocate();
        }

        // try to add to existing containers
        // iterate through pool list to access subpools
        PoolChunk<ElementType, ChunkAllocationSize>* c = chunks.get();
        for (;;)
        {
            ElementType* e = c->allocate();
            if (e != nullptr)
            {
                ++numInUse;
                return e;
            }

            auto next = c->next();
            if (next == nullptr) break;
            else c = next;
        }
        // all existing containers full, create new and add to it
        // TODO check for size here to ensure we are allowed to do this

        combinedCapacity += ChunkAllocationSize;
        ++numInUse;
        c = c->allocateNext();
        return c->allocate();
    }

    void remove(Iterator<ElementType, ChunkAllocationSize>& iterator)
    {
        // TODO
        // Assert instead
        if (iterator.chunk != nullptr)
        {
            iterator.chunk->release(iterator.chunkAccessIndex);
            --numInUse;
        }
    }

    int size()
    {
        return numInUse;
    }

    int capacity()
    {
        return combinedCapacity;
    }

 private:
    int numInUse = 0;
    int combinedCapacity = 0;

    std::unique_ptr<PoolChunk<ElementType, ChunkAllocationSize>> chunks = nullptr;
};

