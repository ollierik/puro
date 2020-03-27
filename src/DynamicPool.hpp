#pragma once 

#include <memory>
#include "Pool.hpp"

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

    template <class ElementType, int ChunkAllocationSize = 32>
    class Iterator
    {
    public:
        Iterator(PoolChunk<ElementType, ChunkAllocationSize>* c)
            : chunk(c)
            , chunkAccessIndex(chunk == nullptr ? -1 : chunk->size()-1)
        {
        }

        Iterator<ElementType, ChunkAllocationSize>& operator*()
        {
            return *this;
        }

        ElementType* operator->()
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

        bool operator!= (const Iterator<ElementType, ChunkAllocationSize>& other)
        {
            return ! (chunkAccessIndex == other.chunkAccessIndex && chunk == other.chunk);
        }

    private:

        friend class DynamicPool<ElementType, ChunkAllocationSize>;

        PoolChunk<ElementType, ChunkAllocationSize>* chunk;
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

    template <class ...Args>
    ElementType* add(Args... args)
    {
        // if pool list hasn't been initialised, create it and add to it
        if (chunks == nullptr)
        {
            combinedCapacity += ChunkAllocationSize;
            ++numInUse;
            chunks = std::make_unique<PoolChunk<ElementType, ChunkAllocationSize>> ();
            return chunks->add(args...);
        }

        // try to add to existing containers
        // iterate through pool list to access subpools
        PoolChunk<ElementType, ChunkAllocationSize>* c = chunks.get();
        for (;;)
        {
            ElementType* e = c->add(args...);
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

        ++combinedCapacity;
        ++numInUse;
        c = c->allocateNext();
        return c->add(args...);
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
