#pragma once

namespace puro {

template <int NumChannels, int Length, typename T = float>
struct stack_block
{
    typedef T value_type;

    inline void assign_allocated(T** dst, int num_channels, int length)
    {
        errorif(length > Length, "requested longer channels than was allocated");

        for (int ch = 0; ch < num_channels; ch++)
        {
            dst[ch] = data[ch];
        }
    }

    T data[NumChannels][Length];
};


template <int NumChannels, int Length, int Alignment=8, typename T = float>
struct aligned_block
{
    typedef T value_type;

    inline void assign_allocated(T** dst, int num_channels, int length)
    {
        errorif(length > Length, "requested longer channels than was allocated");

        for (int ch = 0; ch < num_channels; ch++)
        {
            char* ptr = data[ch];

            size_t remainder = ((size_t)ptr) % Alignment;
            size_t offset = Alignment - remainder;
            dst[ch] = (T*)(ptr + offset);
        }
    }

    char data[NumChannels][sizeof(T) * Length + Alignment - 1];
};
    
template <int Capacity, int ExtraPadding, int Alignment = 8, typename T = float>
struct aligned_fixed_pool
{
    typedef unsigned char byte;
    
    inline aligned_fixed_pool() : ptr(data) {}
    
    inline void assign_allocated(T** dst, int num_channels, int length)
    {
        for (int ch = 0; ch < num_channels; ch++)
        {
            align_ptr(); // ensure that ptr is at correct alignment
            
            const size_t num_bytes_requested = length * sizeof(T);
            //const byte* end = data + sizeof(data);

            errorif (ptr + num_bytes_requested > data + sizeof(data), "out of bounds");

            dst[ch] = (T*)ptr;

            ptr += num_bytes_requested;
        }
        
        //std::cout << "\n";
    }
    
private:
    
    inline void align_ptr()
    {
        const size_t remainder = ((size_t)ptr) % Alignment;
        
        //if (remainder != 0)
            //std::cout << "align pointer by: " << Alignment - remainder << "\n";

        const size_t offset = (remainder == 0) ? 0 : Alignment - remainder;
        ptr += offset;
    }
    
    byte data [sizeof(T) * Capacity + (1 + ExtraPadding) * (Alignment - 1)];
    byte* ptr;
};


template <typename T, typename Allocator>
struct heap_block
{
    heap_block() : num_channels_allocated(-1), num_samples_allocated(-1), ptrs(nullptr) {};

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
            for (int ch = 0; ch < num_channels_allocated; ++ch)
            {
                alloc.deallocate(ptrs[ch], num_samples_allocated);
            }

            // deallocate index
            delete ptrs;
        }
    }

    inline void assign_allocated(T** dst, int num_channels, int length)
    {
        // if not yet allocated
        if (num_channels_allocated == -1 && num_samples_allocated == -1)
        {
            allocate_memory(num_channels, length);
        }

        // if requested memory can't fit the allocated memory
        // heap_block doesn't support reallocation

        errorif (num_channels > num_channels_allocated, "max_channels out of range");
        errorif (length > num_samples_allocated, "too much allocated memory requested");

        for (int ch = 0; ch < num_channels; ch++)
        {
            dst[ch] = ptrs[ch];
        }
    }

    inline void allocate_memory(int num_channels, int num_samples)
    {
        // allocate index
        ptrs = new T* [num_channels];

        // allocate the channel data
        Allocator alloc;
        for (int ch = 0; ch < num_channels; ++ch)
        {
            ptrs[ch] = alloc.allocate(num_samples);
        }

        num_channels_allocated = num_channels;
        num_samples_allocated = num_samples;
    }

    int num_channels_allocated;
    int num_samples_allocated;

    T** ptrs;
};

/**
A pool of heap blocks to own memory used by buffers.
Should be used on initialisation to allocate memory to buffers that are stored in the audio engine.
*/
template <typename T, typename Allocator = std::allocator<T>>
class heap_block_pool
{
public:
    heap_block_pool() : head (nullptr), num_allocated(0) {};

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

    inline void assign_allocated(T** dst, int num_channels, int length)
    {
        allocate_node_and_push_front();
        head->block.assign_allocated(dst, num_channels, length);
    }

private:

    struct linked_node
    {
        heap_block<T, Allocator> block;
        linked_node* next;
    };


    inline void allocate_node_and_push_front()
    {
        linked_node* prev_head = head;
        head = new linked_node();
        head->next = prev_head;
        ++num_allocated;
    }

    linked_node* head;
    int num_allocated;
};




/**
    Works like std::enable_if. Broadcasts type void if type can be used as a memory source for buffers.
    Partial specialisation for the actual types that we want to support.
    Used with fixed_buffer to prevent memory source constructor appearing as a copy constructor.
*/
template <typename T>
struct enable_if_memory_source
{
};

template <int NumChannels, int Length, typename T>
struct enable_if_memory_source<stack_block<NumChannels, Length, T> >
{
    typedef void type;
};
    
template <int NumChannels, int Length, int Alignment, typename T>
struct enable_if_memory_source<aligned_block<NumChannels, Length, Alignment, T> >
{
    typedef void type;
};
    
template <int Capacity, int NumDivisions, int Alignment, typename T>
struct enable_if_memory_source<aligned_fixed_pool<Capacity, NumDivisions, Alignment, T> >
{
    typedef void type;
};

template <typename T, typename Allocator>
struct enable_if_memory_source<heap_block<T, Allocator> >
{
    typedef void type;
};

template <typename T, typename Allocator>
struct enable_if_memory_source<heap_block_pool<T, Allocator> >
{
    typedef void type;
};


} // namespace puro
