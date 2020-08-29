#pragma once

namespace puro {

struct is_memory_source_tag {};


template <int NumChannels, int Length, typename T = float>
struct stack_block : is_memory_source_tag
{
    typedef stack_block<NumChannels, Length, T>& reference_type;

    stack_block()
    {
        for (int ch = 0; ch < NumChannels; ++ch)
        {
            ptrs[ch] = &block[ch * Length];
        }
    }

    //stack_block(const stack_block& copy) = delete;

    T* channel(int ch)
    {
        errorif(ch >= NumChannels, "channel index out of range");
        return ptrs[ch];
    }

    T** get_allocated(int num_channels, int length)
    {
        errorif(num_channels > NumChannels, "requested more channels than was allocated");
        errorif(length > Length, "requested longer channels than was allocated");

        return ptrs;
    }

    T block[NumChannels * Length];
    T* ptrs[NumChannels];
};


template <typename T = float, typename Allocator = std::allocator<T>>
struct heap_block
{
    heap_block() {};

    heap_block(int num_channels, int length)
    {
        allocate_memory(num_channels, length);
    }

    ~heap_block()
    {
        // deallocate channel data
        if (ptrs != nullptr)
        {
            Allocator alloc;
            for (auto ch = 0; ch < num_channels_allocated; ++ch)
            {
                alloc.deallocate(ptrs[ch], num_samples_allocated);
            }

            // deallocate index
            delete ptrs;
        }
    }

    T* channel(int ch)
    {
        errorif(ch >= num_channels_allocated, "channel index out of range");
        return ptrs[ch];
    }

    T** get_allocated(int num_channels, int length)
    {
        // if not yet allocated
        if (num_channels_allocated == -1 && num_samples_allocated == -1)
        {
            allocate_memory(num_channels, length);
        }

        // if requested memory can't fit the allocated memory
        // heap_block doesn't support reallocation
        errorif (num_channels > num_channels_allocated, "too many channels requested");
        errorif (length > num_samples_allocated, "too much allocated memory requested");

        return ptrs;
    }

    void allocate_memory(int num_channels, int num_samples)
    {
        // allocate index
        ptrs = new T* [num_channels];

        // allocate the channel data
        Allocator alloc;
        for (auto ch = 0; ch < num_channels; ++ch)
        {
            ptrs[ch] = alloc.allocate(num_samples);
        }

        num_channels_allocated = num_channels;
        num_samples_allocated = num_samples;
    }

    int num_channels_allocated = -1;
    int num_samples_allocated = -1;

    T** ptrs = nullptr;
};



} // namespace puro
