#pragma once

namespace puro {
    
template <int NumChannels, typename T=float>
struct ring_buffer
{
    // template arg broadcasts
    typedef T value_type;
    
    int num_samples;
    int index = 0;
    std::array<T*, NumChannels> ptrs;

    int length() const noexcept { return num_samples; }
    constexpr int num_channels() const noexcept { return NumChannels; } // some more advanced class may want to redefine this

    T* channel(int ch) const noexcept
    {
        errorif(ch < 0 || ch >= num_channels(), "channel out of range");
        return ptrs[ch];
    }
    
    T* operator[] (int ch)
    {
        errorif(ch < 0 || ch >= num_channels(), "channel out of range");
        return ptrs[ch];
    }
};

    
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename RingBufferType>
RingBufferType ring_buffer_advance_index(RingBufferType ringbuf, int num_samples)
{
    ringbuf.index = math::wrap(ringbuf.index + num_samples);
    return ringbuf;
}
    
template <typename RingBufferType>
void ring_buffer_clear(RingBufferType ringbuf, int offset, int length)
{
    errorif(length > ringbuf.length(), "length parameter exceeds ring buffer length");
    
    const int i0 = math::wrap(ringbuf.index + offset, ringbuf.length());
    const int overflow = i0 + length - ringbuf.length();

    if (overflow > 0) // ringbuffer wraps
    {
        const int n = length - overflow;
        
        for (auto ch=0; ch<ringbuf.num_channels(); ++ch)
        {
            math::clear(&ringbuf[ch][i0], n);
            math::clear(&ringbuf[ch][0], overflow);
        }
    }
    else
    {
        for (auto ch=0; ch<ringbuf.num_channels(); ++ch)
        {
            math::clear(&ringbuf[ch][i0], length);
        }
    }
}
    
template <typename RingBufferType, typename BufferType>
void ring_buffer_add_buffer(RingBufferType dst, BufferType src, int offset)
{
    errorif(src.length() > dst.length(), "src buffer length exceeds ring buffer length");

    const int num_samples_to_add = src.length();
    const int i0 = math::wrap(dst.index + offset, dst.length());
    const int overflow = i0 + num_samples_to_add - dst.length();

    if (overflow > 0) // dstfer wraps
    {
        const int num_samples_first = num_samples_to_add - overflow;

        for (auto ch=0; ch<dst.num_channels(); ++ch)
        {
            math::add(&dst[ch][i0], &src[ch][0], num_samples_first);
            math::add(&dst[ch][0], &src[ch][num_samples_first], overflow);
        }
    }
    else
    {
        for (auto ch=0; ch<dst.num_channels(); ++ch)
        {
            math::add(&dst[ch][i0], &src[ch][0], num_samples_to_add);
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

        for (auto ch=0; ch<dst.num_channels(); ++ch)
        {
            math::copy(&dst[ch][0], &src[ch][i0], num_samples_first);
            math::copy(&dst[ch][num_samples_first], &src[ch][0], overflow);
        }
    }
    else
    {
        for (auto ch=0; ch < dst.num_channels(); ++ch)
        {
            math::copy(&dst[ch][0], &src[ch][i0], num_samples_to_copy);
        }
    }
}
    


/** NOT TESTED
template <typename RingBufferType, typename BufferType>
RingBufferType ring_buffer_write_and_advance(RingBufferType ringbuf, BufferType input)
{
    errorif(ringBuffer.num_channels() != input.num_channels(), "channel configs don't match");
    errorif(ringBuffer.length() < input.length(), "input shouldn't be longer than ring buffer");

    const int overflow = ringBuffer.writeIndex + input.length() - ringBuffer.length();

    // ring buffer wraps around
    if (overflow > 0)
    {
        const int length = input.length() - overflow;

        for (int ch=0; ch<ringBuffer.getNumChannels(); ++ch)
        {
            math::copy(&ringBuffer.channel(ch)[index], input.channel(ch), length);
            math::copy(&ringBuffer.channel(ch)[0], &input.channel(ch)[length], overflow);
        }
    }
    else
    {
        for (int ch=0; ch<ringBuffer.getNumChannels(); ++ch)
        {
            math::copy(&ringBuffer.channel(ch)[index], input.channel(ch), input.size());
        }
    }
    ringbuffer_advance_write_index(ringBuffer, input.length());
    return ringBuffer;
}
 */

    
    
    
} // namespace puro
