
#pragma once

namespace puro {

template <int NumChannels, int Length, typename T>
struct fixed_buffer;
    
template <int NumChannels, typename T>
struct buffer;


template <int NumChannels, int Length, typename T = float>
struct fixed_buffer
{
    typedef T value_type;
    
    T* ptrs [NumChannels] = { 0 };
    
    constexpr static int num_channels() { return NumChannels; }
    constexpr static int length() { return Length; }

    inline T* channel(int ch) const
    {
        errorif(ch < 0 || ch >= this->num_channels(), "channel out of range");
        return ptrs[ch];
    }
    
    inline void clear() const
    {
        for (int ch=0; ch < NumChannels; ++ch)
        {
            math::clear(channel(ch), Length);
        }
    }

    inline buffer<NumChannels, T> sub (int offset, int n) const
    {
        errorif(offset < 0, "offset below zero");
        errorif(offset > length(), "offset greater than number of samples available");
        errorif(n < 0, "negative length");
        errorif(offset + n > length(), "sub end exceeds the number of samples available");
        
        buffer<NumChannels, T> retbuf (n);
        
        for (int ch = 0; ch < num_channels(); ++ch)
            retbuf.ptrs[ch] = &ptrs[ch][offset];
        
        return retbuf;
    }
    
    /**
    Return fixed_buffer with constant length N and runtime offset
     */
    template <int N>
    inline fixed_buffer<NumChannels, N, T> sub (int offset) const
    {
        errorif(offset < 0, "offset below zero");
        errorif(offset > length(), "offset greater than number of samples available");
        errorif(N < 0, "negative length");
        errorif(offset + N > length(), "sub end exceeds the number of samples available");
        
        fixed_buffer<NumChannels, N> fixbuf;
        
        for (int ch=0; ch < NumChannels; ++ch)
        {
            fixbuf.ptrs[ch] = &ptrs[ch][offset];
        }
        
        return fixbuf;
    }
    
    /**
     Return fixed_buffer with constant offset and constant length
     */
    template <int Offset, int N>
    inline fixed_buffer<NumChannels, N, T> sub () const
    {
        errorif(Offset < 0, "offset below zero");
        errorif(Offset > length(), "offset greater than number of samples available");
        errorif(N < 0, "negative length");
        errorif(Offset + N > length(), "sub end exceeds the number of samples available");
        
        fixed_buffer<NumChannels, N> fixbuf;
        
        for (int ch=0; ch < NumChannels; ++ch)
        {
            fixbuf.ptrs[ch] = &ptrs[ch][Offset];
        }
        
        return fixbuf;
    }
    
    /// Return truncated fixed_buffer with length N
    template <int N>
    inline fixed_buffer<NumChannels, N, T> trunc() const
    {
        errorif(N < 0, "negative length");
        errorif(N > length(), "cut end exceeds the number of samples available");
        
        fixed_buffer<NumChannels, N> fixbuf;
        
        for (int ch=0; ch < NumChannels; ++ch)
        {
            fixbuf.ptrs[ch] = &ptrs[ch][0];
        }
        
        return fixbuf;
    }
    
    inline fixed_buffer<1, Length, T> mono (int ch) const
    {
        errorif (ch >= num_channels(), "channel out of bounds");
        
        fixed_buffer<1, Length> fixbuf;
        fixbuf.ptrs[0] = ptrs[ch];
        return fixbuf;
    }
    
    inline buffer<NumChannels, T> as_buffer() const
    {
        buffer<NumChannels, T> buf (Length, ptrs);
        return buf;
    }


    // ctors
    
    inline fixed_buffer() {};

    inline fixed_buffer (const fixed_buffer& other)
    {
        for (int ch = 0; ch < num_channels(); ++ch)
            ptrs[ch] = other.ptrs[ch];
    }

    inline fixed_buffer(T** channelPtrs)
    {
        for (int ch = 0; ch < num_channels(); ++ch)
            ptrs[ch] = channelPtrs[ch];
    }


    template <typename MemorySource>
    inline fixed_buffer (MemorySource& ms, typename enable_if_memory_source<MemorySource>::type* dummy = 0)
    {
        ms.assign_allocated(ptrs, NumChannels, Length);
    }
};
    


template <int NumChannels, typename T = float>
struct buffer
{
    typedef T value_type;
    
    int num_samples = 0;
    T* ptrs [NumChannels] = { 0 };

    constexpr static inline int num_channels() { return NumChannels; }
    inline int length() const { return num_samples; }
    
    inline void clear() const
    {
        for (int ch=0; ch < NumChannels; ++ch)
        {
            math::clear(channel(ch), num_samples);
        }
    }

    inline T* channel(int ch) const
    {
        errorif(ch < 0 || ch >= this->num_channels(), "channel out of range");
        return ptrs[ch];
    }

    inline buffer sub (int offset, int n) const
    {
        errorif(offset < 0, "offset below zero");
        errorif(offset > length(), "offset greater than number of samples available");
        errorif(n < 0, "negative length");
        errorif(offset + n > length(), "sub end exceeds the number of samples available");
        
        buffer retbuf (n);
        
        for (int ch = 0; ch < num_channels(); ++ch)
            retbuf.ptrs[ch] = &ptrs[ch][offset];
        
        return retbuf;
    }
    
    /// Return fixed_buffer with constant length N
    template <int N>
    inline fixed_buffer<NumChannels, N, T> sub (int offset = 0) const
    {
        errorif(offset < 0, "offset below zero");
        errorif(offset > length(), "offset greater than number of samples available");
        errorif(N < 0, "negative length");
        errorif(offset + N > length(), "sub end exceeds the number of samples available");
        
        fixed_buffer<NumChannels, N> fixbuf;
        
        for (int ch=0; ch < NumChannels; ++ch)
        {
            fixbuf.ptrs[ch] = &ptrs[ch][offset];
        }
        
        return fixbuf;
    }
    
    /// Return fixed_buffer with constant offset and length
    template <int Offset, int N>
    inline fixed_buffer<NumChannels, N, T> sub () const
    {
        errorif(Offset < 0, "offset below zero");
        errorif(Offset > length(), "offset greater than number of samples available");
        errorif(N < 0, "negative length");
        errorif(Offset + N > length(), "sub end exceeds the number of samples available");
        
        fixed_buffer<NumChannels, N> fixbuf;
        
        for (int ch=0; ch < NumChannels; ++ch)
        {
            fixbuf.ptrs[ch] = &ptrs[ch][Offset];
        }
        
        return fixbuf;
    }
    
    inline buffer trunc (int length)
    {
        errorif (length > num_samples, "Requested length longer than current length");
        return buffer (length, ptrs);
    }
    
    inline buffer tail (int offset)
    {
        errorif (offset > num_samples, "Requested offset longer than current length");
        return sub(offset, num_samples - offset);
    }
    
    inline buffer slice (int start, int end)
    {
        errorif (start > end, "start is not smaller than end");
        errorif (start < 0, "start is negative");
        errorif (end > length(), "end exceeds length");
        
        return sub (start, end - start);
    }

    inline buffer <1, T> mono (int ch) const
    {
        errorif (ch >= num_channels(), "channel out of bounds");
        
        buffer<1, T> buf (length());
        buf.ptrs[0] = ptrs[ch];
        return buf;
    }

    // ctors
    
    inline buffer() {};
    
    inline buffer(int length) : num_samples(length) {};

    inline buffer(int length, T* const * channel_ptrs) : num_samples(length)
    {
        for (int ch = 0; ch < num_channels(); ++ch)
            ptrs[ch] = channel_ptrs[ch];
    }
    
    inline buffer(int length, T* const (&channel_ptrs) [NumChannels]) : num_samples(length)
    {
        for (int ch = 0; ch < num_channels(); ++ch)
            ptrs[ch] = channel_ptrs[ch];
    }

    template <typename MemorySource>
    inline buffer (int length, MemorySource& ms,
                   typename enable_if_memory_source<MemorySource>::type* dummy = 0)
                   : num_samples(length)
    {
        ms.assign_allocated(ptrs, NumChannels, num_samples);
    }

    inline buffer (int length, std::vector<value_type>& vec)
    : num_samples(length)
    {
        const int length_required = length * num_channels();
        if (vec.size() < length_required)
        {
            vec.resize(length_required);
        }
        
        for (int ch = 0; ch < num_channels(); ++ch)
            ptrs[ch] = &vec[ch * length];
    }
};
    

    
    
    
template <int MaxNumChannels, typename T = float>
struct dynamic_buffer
{
    typedef T value_type;

    int num_chs = 0;
    int num_samples = 0;
    T* ptrs [MaxNumChannels] = { 0 };

    inline int num_channels() const { return num_chs; }
    inline int length() const { return num_samples; }

    inline void clear() const
    {
        for (int ch=0; ch < num_channels(); ++ch)
        {
            math::clear(channel(ch), num_samples);
        }
    }

    inline T* channel(int ch) const
    {
        errorif(ch < 0 || ch >= this->num_channels(), "channel out of range");
        return ptrs[ch];
    }

    inline dynamic_buffer sub (int offset, int n) const
    {
        errorif(offset < 0, "offset below zero");
        errorif(offset > length(), "offset greater than number of samples available");
        errorif(n < 0, "negative length");
        errorif(offset + n > length(), "sub end exceeds the number of samples available");

        dynamic_buffer retbuf (num_channels(), n);

        for (int ch = 0; ch < num_channels(); ++ch)
            retbuf.ptrs[ch] = &ptrs[ch][offset];

        return retbuf;
    }

    inline buffer <1, T> mono (int ch) const
    {
        errorif (ch >= num_channels(), "channel out of bounds");

        buffer<1, T> buf (length());
        buf.ptrs[0] = ptrs[ch];
        return buf;
    }
    
    template <typename BufferType>
    inline BufferType as_buffer()
    {
        BufferType retbuf (num_samples, ptrs);
        
        errorif(retbuf.num_channels() > num_channels(), "Requested more channels than current dynamic_buffer contains");
        
        return retbuf;
    }

    // ctors

    inline dynamic_buffer() {};

    inline dynamic_buffer(int num_channels, int length) : num_chs(num_channels), num_samples(length) {};

    inline dynamic_buffer(int num_channels, int length, T* const * channel_ptrs) : num_chs(num_channels), num_samples(length)
    {
        for (int ch = 0; ch < num_chs; ++ch)
            ptrs[ch] = channel_ptrs[ch];
    }

    template <typename MemorySource>
    inline dynamic_buffer (int num_channels, int length, MemorySource& ms,
                   typename enable_if_memory_source<MemorySource>::type* dummy = 0)
    : num_chs(num_channels), num_samples(length)
    {
        ms.assign_allocated(ptrs, num_chs, num_samples);
    }
};

} // namespace puro

