#pragma once

namespace puro {
    
    
    /*
// fixed channels, fixed length
template <int NumChannels, int Length, typename T>
struct buffer_base
{
    constexpr static int num_channels() { return NumChannels; }
    constexpr static int length() { return Length; }
};
    
// fixed channels, dynamic length
template <int NumChannels, typename T>
struct buffer_base <NumChannels, -1, T>
{
    int length;
    
    constexpr static int num_channels() { return NumChannels; }
    int length() const { return num_samples; }
};
     */

template <int NumChannels, int Length, typename T>
struct fixed_buffer;
    
template <int NumChannels, typename T>
struct buffer;


// fixed
template <int NumChannels, int Length, typename T = float>
struct fixed_buffer
{
    typedef T value_type;
    constexpr static auto max_channels = NumChannels;
    
    std::array<T*, NumChannels> ptrs;
    
    constexpr static int num_channels() { return NumChannels; }
    constexpr static int length() { return Length; }

    T* operator[] (int ch) const
    {
        errorif(ch < 0 || ch >= this->num_channels(), "channel out of range");
        return ptrs[ch];
    }
    
    T* channel(int ch) const
    {
        errorif(ch < 0 || ch >= this->num_channels(), "channel out of range");
        return ptrs[ch];
    }

    buffer<NumChannels, T> slice (int start, int end)
    {
        errorif(start < 0, "slice start below zero");
        errorif(start > length(), "slice start greater than number of samples available");
        errorif(end < start, "slice end below start");
        errorif(end > length(), "slice end greater than number of samples available");
        
        buffer<NumChannels, T> retbuf (end - start);
        
        for (int ch = 0; ch < num_channels(); ++ch)
            retbuf.ptrs[ch] = &ptrs[ch][start];
        
        return retbuf;
    }
    
    template <int NumSamples>
    fixed_buffer<NumChannels, NumSamples, T> slice (int offset = 0)
    {
        fixed_buffer<NumChannels, NumSamples> fixbuf;
        
        for (auto ch=0; ch < NumChannels; ++ch)
        {
            fixbuf.ptrs[ch] = &ptrs[ch][offset];
        }
        
        return fixbuf;
    }
    
    // ctors
    fixed_buffer() = default;
    fixed_buffer(fixed_buffer&) = default;

    fixed_buffer(T** channelPtrs)
    {
        for (auto ch = 0; ch < num_channels(); ++ch)
            ptrs[ch] = channelPtrs[ch];
    }

    // todo enable if
    template <typename MemorySource>
    fixed_buffer (MemorySource& as, void* dummy = nullptr) // second parameter changes signature to not appear as copy ctor
    {
        T** data = as.get_allocated(NumChannels, Length);

        for (int ch=0; ch<num_channels(); ++ch)
        {
            ptrs[ch] = data[ch];
        }
    }
};
    

/*
template <int NumChannels, int Length, typename T = float>
fixed_buffer<NumChannels, Length, T> make_fixed(stack_memory<NumChannels, Length>& memory)
{
    fixed_buffer<NumChannels, Length, T> buf;
    for (auto ch = 0; ch < NumChannels; ++ch)
    {
        buf.ptrs[ch] = memory.channel(ch);
    }
    return buf;
}
    
template <int NumChannels, int Length, typename T, typename Allocator>
fixed_buffer<NumChannels, Length, T> make_fixed(heap_block<NumChannels, T, Allocator>& memory)
{
    fixed_buffer<NumChannels, Length, T> buf;
    for (auto ch = 0; ch < NumChannels; ++ch)
    {
        buf.ptrs[ch] = memory.channel(ch);
    }
    return buf;
}
*/
    

template <int NumChannels, typename T = float>
struct buffer
{
    typedef T value_type;
    constexpr static auto max_channels = NumChannels;
    
    int num_samples;
    std::array<T*, NumChannels> ptrs;

    constexpr static int num_channels() { return NumChannels; }
    int length() const { return num_samples; }
    
    void clear()
    {
        buffer_clear(*this);
    }

    T* operator[] (int ch) const
    {
        errorif(ch < 0 || ch >= this->num_channels(), "channel out of range");
        return ptrs[ch];
    }

    T* channel(int ch) const
    {
        errorif(ch < 0 || ch >= this->num_channels(), "channel out of range");
        return ptrs[ch];
    }

    buffer slice (int start, int end)
    {
        errorif(start < 0, "slice start below zero");
        errorif(start > length(), "slice start greater than number of samples available");
        errorif(end < start, "slice end below start");
        errorif(end > length(), "slice end greater than number of samples available");
        
        buffer retbuf (end - start);
        
        for (int ch = 0; ch < num_channels(); ++ch)
            retbuf.ptrs[ch] = &ptrs[ch][start];
        
        return retbuf;
    }
    
    template <int NumSamples>
    fixed_buffer<NumChannels, NumSamples, T> slice (int offset = 0)
    {
        fixed_buffer<NumChannels, NumSamples> fixbuf;
        
        for (auto ch=0; ch < NumChannels; ++ch)
        {
            fixbuf.ptrs[ch] = &ptrs[ch][offset];
        }
        
        return fixbuf;
    }

    // ctors
    buffer(int length) : num_samples(length) {};

    buffer(int length, T** channel_ptrs) : num_samples(length)
    {
        for (auto ch = 0; ch < num_channels(); ++ch)
            ptrs[ch] = channel_ptrs[ch];
    }

    template <typename AllocatorSource>
    buffer (int length, AllocatorSource& as) : num_samples(length)
    {
        T** data = as.get_allocated(NumChannels, num_samples);

        for (int ch=0; ch<num_channels(); ++ch)
        {
            ptrs[ch] = data[ch];
        }
    }
    
    /*
    template <int MemoryNumChannels, int N>
    buffer (const stack_memory<MemoryNumChannels, N, T>& memory)
    {
        for (auto ch = 0; ch < num_channels(); ++ch)
            ptrs[ch] = memory[ch].data();
    }
    
    template <int MemoryNumChannels, int N>
    buffer (const heap_memory<MemoryNumChannels, N, T>& memory)
    {
        for (auto ch = 0; ch < num_channels(); ++ch)
            ptrs[ch] = memory[ch].data();
    }
     */
};
    
    
template <class BufferType, class Allocator = std::allocator<typename BufferType::value_type>>
struct mem_buffer : public BufferType
{
    typedef typename BufferType::value_type value_type;

    mem_buffer(mem_buffer &&) = delete;
    mem_buffer(const mem_buffer &) = delete;
    
    mem_buffer() : BufferType()
    {
        for (auto ch=0; ch < this->num_channels(); ++ch)
        {
            vectors[ch].resize(BufferType::length());
            this->ptrs[ch] = vectors[ch].data();
        }
    }

    mem_buffer(int length) : BufferType(length)
    {
        for (auto ch=0; ch < this->num_channels(); ++ch)
        {
            vectors[ch].resize(BufferType::length());
            this->ptrs[ch] = vectors[ch].data();
        }
    }

    BufferType operator* () { return *this; }
    BufferType get() { return *this; }

    void operator= (BufferType new_buffer)
    {
        *reinterpret_cast<BufferType*>(this) = new_buffer;
    }

    std::array<std::vector<value_type, Allocator>, BufferType::max_channels> vectors;
};

} // namespace puro
