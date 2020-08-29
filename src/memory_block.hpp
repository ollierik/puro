#pragma once

namespace puro {


template <int NumChannels, int Length, typename T = float>
struct stack_block
{
    //typedef void test_type;

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

/**
A pool of heap blocks to own memory used by buffers.
Should be used on initialisation to allocate memory to buffers that are stored in the audio engine.
*/
template <typename T = float, typename Allocator = std::allocator<T>>
class heap_block_pool
{
public:
    heap_block_pool() {};

    ~heap_block_pool()
    {
        int num_deleted = 0;
        while (head != nullptr)
        {
            linked_node* prev_head = head;
            head = prev_head->next;
            delete prev_head;
            ++num_deleted;
        }

        errorif(num_deleted != num_allocated, "heap_block_pool leaking memory");
    }

    T** get_allocated(int num_channels, int length)
    {
        allocate_and_push_front();
        return head->block.get_allocated(num_channels, length);
    }

private:

    struct linked_node
    {
        heap_block<T, Allocator> block;
        linked_node* next;
    };


    void allocate_and_push_front()
    {
        linked_node* prev_head = head;
        head = new linked_node();
        head->next = prev_head;
        ++num_allocated;
    }

    linked_node* head = nullptr;
    int num_allocated = 0;
};




/**
    Works like std::enable_if. Broadcasts type void if type can be used as a memory source for buffers.
    Partial specialisation for the actual types that we want to support.
    Used with fixed_buffer to prevent memory source constructor appearing as a copy constructor.
*/
template <typename T>
struct is_memory_source
{
};

template <int NumChannels, int Length, typename T>
struct is_memory_source<stack_block<NumChannels, Length, T>>
{
    typedef void type;
};

template <typename T, typename Allocator>
struct is_memory_source<heap_block<T, Allocator>>
{
    typedef void type;
};

template <typename T, typename Allocator>
struct is_memory_source<heap_block_pool<T, Allocator>>
{
    typedef void type;
};


} // namespace puro
