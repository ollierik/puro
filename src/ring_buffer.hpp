#pragma once

namespace puro {
    
template <typename BT>
struct ring_buffer_parts
{
    BT b0;
    BT b1;
    inline ring_buffer_parts(BT b0, BT b1) : b0(b0), b1(b1) {}
    inline bool has_second() { return b1.length() > 0; }
};

template <int NumChannels, typename T = float>
struct ring_buffer
{
    typedef T value_type;
    typedef buffer<NumChannels, T> buffer_type;
    typedef ring_buffer_parts <buffer<NumChannels, T> > parts_type;

    int num_samples;
    int index;
    T* ptrs [NumChannels];

    inline int length() const { return num_samples; }
    static inline int num_channels() { return NumChannels; } // some more advanced class may want to redefine this

    T* channel(int ch) const
    {
        errorif(ch < 0 || ch >= num_channels(), "channel out of range");
        return ptrs[ch];
    }
    
    /**
     Return ring_buffer as buffer.
     No wrapping or index offsetting here, caller is in charge of ensuring that we won't end up out-of-bounds.
     */
    buffer_type raw_buffer (int offset, int length)
    {
        buffer_type buf (length);
        for (int ch=0; ch<NumChannels; ++ch)
        {
            buf.ptrs[ch] = &ptrs[ch][offset];
        }
        return buf;
    }

    // ctors
    inline ring_buffer() : num_samples(0), index(0) {};
    inline ring_buffer(int length) : num_samples(length), index(0) {};
    inline ring_buffer(int length, T** channelPtrs) : num_samples(length)
    {
        for (int ch = 0; ch < num_channels(); ++ch)
            ptrs[ch] = channelPtrs[ch];
    }
    
    template <typename MemorySource>
    inline ring_buffer (int length, MemorySource& ms) : num_samples(length), index(0)
    {
        ms.assign_allocated(ptrs, NumChannels, num_samples);
    }
    
};

    
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename RingBufferType>
inline RingBufferType ring_buffer_advance_index(RingBufferType ringbuf, int num_samples)
{
    ringbuf.index = math::wrap(ringbuf.index + num_samples, ringbuf.length());
    return ringbuf;
}

/**
 Get a number of samples from buffer. If buffer wraps, returns two parts.
 */
template <typename RBT>
ring_buffer_parts<typename RBT::buffer_type>
ring_buffer_get_parts(RBT ringbuf, int offset, int length)
{
    errorif(length > ringbuf.length(), "length parameter exceeds ring buffer length");
    
    typedef typename RBT::buffer_type BT;
    typedef typename RBT::parts_type PT;

    const int i0 = math::wrap(ringbuf.index + offset, ringbuf.length());
    const int overflow = i0 + length - ringbuf.length();

    if (overflow > 0) // ringbuffer wraps
    {
        const int num_samples_first = length - overflow;

        BT b0 = ringbuf.raw_buffer (i0, num_samples_first);
        BT b1 = ringbuf.raw_buffer (0, overflow);
        
        return PT (b0, b1);
    }
    else
    {
        BT b0 = ringbuf.raw_buffer (i0, length);
        BT b1 = BT (0);
        return PT (b0, b1);
    }
}
    
template <typename RBT>
void ring_buffer_clear(RBT ringbuf, int offset, int length)
{
    typedef typename RBT::parts_type PT;
    PT parts = ring_buffer_get_parts(ringbuf, offset, length);

    parts.b0.clear();
    
    if (parts.has_second())
    {
        parts.b1.clear();
    }
}
    
template <typename RBT, typename BT>
void ring_buffer_add_from_buffer(RBT dst, BT src, int offset)
{
    typedef typename RBT::parts_type PT;
    PT parts = ring_buffer_get_parts(dst, offset, src.length());
    
    add(parts.b0, src.sub(0, parts.b0.length() ));
    
    if (parts.has_second())
    {
        add(parts.b1, src.sub(parts.b0.length(), parts.b1.length()));
    }
}
    
template <typename RBT, typename BT>
void ring_buffer_copy_from_buffer(RBT dst, BT src, int offset)
{
    typedef typename RBT::parts_type PT;
    PT parts = ring_buffer_get_parts(dst, offset, src.length());
    
    copy(parts.b0, src.sub(0, parts.b0.length() ));
    
    if (parts.has_second())
    {
        copy(parts.b1, src.sub(parts.b0.length(), parts.b1.length()));
    }
}
    
template <typename RBT, typename BT>
void ring_buffer_copy_from_buffer_downmixing(RBT dst, BT src, int offset)
{
    typedef typename RBT::parts_type PT;
    PT parts = ring_buffer_get_parts(dst, offset, src.length());
    
    copy_downmixing(parts.b0, src.sub(0, parts.b0.length() ));
    
    if (parts.has_second())
    {
        copy_downmixing(parts.b1, src.sub(parts.b0.length(), parts.b1.length()));
    }
}
    
template <typename RBT, typename BT>
void ring_buffer_copy_to_buffer(BT dst, RBT src, int offset)
{
    typedef typename RBT::parts_type PT;
    PT parts = ring_buffer_get_parts(src, offset, dst.length());
    
    copy(dst.sub(0, parts.b0.length()), parts.b0);
    
    if (parts.has_second())
    {
        copy(dst.sub(parts.b0.length(), parts.b1.length()), parts.b1);
    }
}
    
template <typename RBT, typename BT>
void ring_buffer_add_to_buffer(BT dst, RBT src, int offset)
{
    typedef typename RBT::parts_type PT;
    PT parts = ring_buffer_get_parts(src, offset, dst.length());
    
    add(dst.sub(0, parts.b0.length()), parts.b0);
    
    if (parts.has_second())
    {
        add(dst.sub(parts.b0.length(), parts.b1.length()), parts.b1);
    }
}

template <typename RBT, typename BT>
void ring_buffer_multiply_add_to_buffer(BT dst, RBT src, int offset, float multiplier)
{
    typedef typename RBT::parts_type PT;
    PT parts = ring_buffer_get_parts(src, offset, dst.length());

    multiply_add(dst.sub(0, parts.b0.length()), parts.b0, multiplier);

    if (parts.has_second())
    {
        multiply_add(dst.sub(parts.b0.length(), parts.b1.length()), parts.b1, multiplier);
    }
}
    

template <typename RingBufferType, typename BufferType>
void ring_buffer_copy_to_buffer_decimating(BufferType dst, RingBufferType src, int offset, int stride)
{
    errorif(dst.length() > src.length(), "dst length exceeds ring buffer length");
    errorif(src.length() % stride != 0, "ring buffer length should be divisable with stride");
    errorif(src.index % stride != 0, "ring buffer index is not aligned with stride");

    const int num_samples_to_copy = dst.length() * stride; // without stride
    const int i0 = math::wrap(src.index + offset, src.length());
    const int overflow = i0 + dst.length() - src.length();

    if (overflow > 0) // ringbuffer wraps
    {
        const int num_samples_first = num_samples_to_copy - overflow;

        for (int ch=0; ch<dst.num_channels(); ++ch)
        {
            math::copy_decimating(&dst.channel(ch)[0], &src.channel(ch)[i0], stride, num_samples_first);
            math::copy_decimating(&dst.channel(ch)[num_samples_first], &src.channel(ch)[0], stride, overflow);
        }
    }
    else
    {
        for (int ch=0; ch < dst.num_channels(); ++ch)
        {
            math::copy_decimating(&dst.channel(ch)[0], &src.channel(ch)[i0], stride, num_samples_to_copy);
        }
    }
}
    
/* The original implementations, disabled for now. */
#if 0 //////////////////////////////////////////////////////////////////////////////////
    
    template <typename RBT, typename BT>
    void ring_buffer_copy_to_buffer_decimating(BT dst, RBT src, int offset, int stride)
    {
        typedef typename RBT::parts_type PT;
        PT parts = ring_buffer_get_parts(src, offset, dst.length() * stride);
        
        copy_decimating(dst.sub(0, parts.b0.length()/stride), parts.b0, stride);
        
        if (parts.has_second())
        {
            copy_decimating(dst.sub(parts.b0.length()/stride, parts.b1.length()/stride), parts.b1, stride);
        }
    }
    
    
    
    template <typename RBT>
    void ring_buffer_clear(RBT ringbuf, int offset, int length)
    {
        errorif(length > ringbuf.length(), "length parameter exceeds ring buffer length");
        
        const int i0 = math::wrap(ringbuf.index + offset, ringbuf.length());
        const int overflow = i0 + length - ringbuf.length();
        
        if (overflow > 0) // ringbuffer wraps
        {
            const int num_samples_first = length - overflow;
            
            for (int ch=0; ch<ringbuf.num_channels(); ++ch)
            {
                math::clear(&ringbuf.channel(ch)[i0], num_samples_first);
                math::clear(&ringbuf.channel(ch)[0], overflow);
            }
        }
        else
        {
            for (int ch=0; ch<ringbuf.num_channels(); ++ch)
            {
                math::clear(&ringbuf.channel(ch)[i0], length);
            }
        }
    }
    
    
    
    template <typename RingBufferType, typename BufferType>
    void ring_buffer_add_from_buffer(RingBufferType dst, BufferType src, int offset)
    {
        errorif(src.length() > dst.length(), "src buffer length exceeds ring buffer length");
        
        const int num_samples_to_add = src.length();
        const int i0 = math::wrap(dst.index + offset, dst.length());
        const int overflow = i0 + num_samples_to_add - dst.length();
        
        if (overflow > 0) // wraps
        {
            const int num_samples_first = num_samples_to_add - overflow;
            
            for (int ch=0; ch<dst.num_channels(); ++ch)
            {
                math::add(&dst.channel(ch)[i0], &src.channel(ch)[0], num_samples_first);
                math::add(&dst.channel(ch)[0], &src.channel(ch)[num_samples_first], overflow);
            }
        }
        else
        {
            for (int ch=0; ch<dst.num_channels(); ++ch)
            {
                math::add(&dst.channel(ch)[i0], &src.channel(ch)[0], num_samples_to_add);
            }
        }
    }
    
    
    
    template <typename RingBufferType, typename BufferType>
    void ring_buffer_copy_from_buffer(RingBufferType dst, BufferType src, int offset)
    {
        errorif(src.length() > dst.length(), "src buffer length exceeds ring buffer length");
        
        const int num_samples_to_add = src.length();
        const int i0 = math::wrap(dst.index + offset, dst.length());
        const int overflow = i0 + num_samples_to_add - dst.length();
        
        if (overflow > 0) // wraps
        {
            const int num_samples_first = num_samples_to_add - overflow;
            
            for (int ch=0; ch<dst.num_channels(); ++ch)
            {
                math::copy(&dst.channel(ch)[i0], &src.channel(ch)[0], num_samples_first);
                math::copy(&dst.channel(ch)[0], &src.channel(ch)[num_samples_first], overflow);
            }
        }
        else
        {
            for (int ch=0; ch<dst.num_channels(); ++ch)
            {
                math::copy(&dst.channel(ch)[i0], &src.channel(ch)[0], num_samples_to_add);
            }
        }
    }
    
    
    template <typename RingBufferType, typename BufferType>
    void ring_buffer_copy_from_buffer_downmixing(RingBufferType dst, BufferType src, int offset)
    {
        errorif(dst.num_channels() != 1, "expexting dst to be mono ring buffer");
        errorif(src.length() > dst.length(), "src buffer length exceeds ring buffer length");
        
        const int num_samples_to_add = src.length();
        const int i0 = math::wrap(dst.index + offset, dst.length());
        const int overflow = i0 + num_samples_to_add - dst.length();
        
        if (overflow > 0) // wraps
        {
            const int num_samples_first = num_samples_to_add - overflow;
            
            math::copy(&dst.channel(0)[i0], &src.channel(0)[0], num_samples_first);
            math::copy(&dst.channel(0)[0], &src.channel(0)[num_samples_first], overflow);
            
            for (int ch=1; ch<dst.num_channels(); ++ch)
            {
                math::add(&dst.channel(0)[i0], &src.channel(ch)[0], num_samples_first);
                math::add(&dst.channel(0)[0], &src.channel(ch)[num_samples_first], overflow);
            }
        }
        else
        {
            math::copy(&dst.channel(0)[i0], &src.channel(0)[0], num_samples_to_add);
            
            for (int ch=1; ch<dst.num_channels(); ++ch)
            {
                math::copy(&dst.channel(0)[i0], &src.channel(ch)[0], num_samples_to_add);
            }
        }
    }
    
    
    template <typename RingBufferType, typename BufferType>
    void ring_buffer_copy_to_buffer(BufferType dst, RingBufferType src, int offset)
    {
        errorif(dst.length() > src.length(), "dst length exceeds ring buffer length");
        
        const int num_samples_to_copy = dst.length();
        const int i0 = math::wrap(src.index + offset, src.length());
        const int overflow = i0 + dst.length() - src.length();
        
        if (overflow > 0) // ringbuffer wraps
        {
            const int num_samples_first = num_samples_to_copy - overflow;
            
            for (int ch=0; ch<dst.num_channels(); ++ch)
            {
                math::copy(&dst.channel(ch)[0], &src.channel(ch)[i0], num_samples_first);
                math::copy(&dst.channel(ch)[num_samples_first], &src.channel(ch)[0], overflow);
            }
        }
        else
        {
            for (int ch=0; ch < dst.num_channels(); ++ch)
            {
                math::copy(&dst.channel(ch)[0], &src.channel(ch)[i0], num_samples_to_copy);
            }
        }
    }
    
    template <typename RBT, typename BT>
    void ring_buffer_add_to_buffer(BT dst, RBT src, int offset)
    {
        errorif(dst.length() > src.length(), "dst length exceeds ring buffer length");
        
        const int num_samples_to_copy = dst.length();
        const int i0 = math::wrap(src.index + offset, src.length());
        const int overflow = i0 + dst.length() - src.length();
        
        if (overflow > 0) // ringbuffer wraps
        {
            const int num_samples_first = num_samples_to_copy - overflow;
            
            for (int ch=0; ch<dst.num_channels(); ++ch)
            {
                math::add(&dst.channel(ch)[0], &src.channel(ch)[i0], num_samples_first);
                math::add(&dst.channel(ch)[num_samples_first], &src.channel(ch)[0], overflow);
            }
        }
        else
        {
            for (int ch=0; ch < dst.num_channels(); ++ch)
            {
                math::add(&dst.channel(ch)[0], &src.channel(ch)[i0], num_samples_to_copy);
            }
        }
    }
    
#endif //////////////////////////////////////////////////////////////////////////////////
    


} // namespace puro
