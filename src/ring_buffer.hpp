#pragma once

namespace puro {
    
template <int NumChannels, typename T=float>
struct ring_buffer
{
    typedef T value_type;
    static constexpr int number_of_channels = NumChannels;
    
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
    
    T* operator[] (int ch) const noexcept
    {
        errorif(ch < 0 || ch >= num_channels(), "channel out of range");
        return ptrs[ch];
    }
    
    // ctors
    ring_buffer() = default;
    ring_buffer(int length) : num_samples(length) {};
    ring_buffer(int length, T** channelPtrs) : num_samples(length)
    {
        for (auto ch = 0; ch < num_channels(); ++ch)
            ptrs[ch] = channelPtrs[ch];
    }
};

    
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename RingBufferType>
void ring_buffer_advance_index(RingBufferType& ringbuf, int num_samples)
{
    ringbuf.index = math::wrap(ringbuf.index + num_samples, ringbuf.length());
    //return ringbuf;
}
    
template <typename RingBufferType>
void ring_buffer_clear(RingBufferType& ringbuf, int offset, int length)
{
    errorif(length > ringbuf.length(), "length parameter exceeds ring buffer length");
    
    const int i0 = math::wrap(ringbuf.index + offset, ringbuf.length());
    const int overflow = i0 + length - ringbuf.length();

    if (overflow > 0) // ringbuffer wraps
    {
        const int num_samples_first = length - overflow;
        
        for (auto ch=0; ch<ringbuf.num_channels(); ++ch)
        {
            math::clear(&ringbuf[ch][i0], num_samples_first);
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
void ring_buffer_add_buffer(RingBufferType& dst, const BufferType& src, int offset)
{
    errorif(src.length() > dst.length(), "src buffer length exceeds ring buffer length");

    const int num_samples_to_add = src.length();
    const int i0 = math::wrap(dst.index + offset, dst.length());
    const int overflow = i0 + num_samples_to_add - dst.length();

    if (overflow > 0) // wraps
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
void ring_buffer_copy_buffer(RingBufferType& dst, const BufferType& src, int offset)
{
    errorif(src.length() > dst.length(), "src buffer length exceeds ring buffer length");

    const int num_samples_to_add = src.length();
    const int i0 = math::wrap(dst.index + offset, dst.length());
    const int overflow = i0 + num_samples_to_add - dst.length();

    if (overflow > 0) // wraps
    {
        const int num_samples_first = num_samples_to_add - overflow;

        for (auto ch=0; ch<dst.num_channels(); ++ch)
        {
            math::copy(&dst[ch][i0], &src[ch][0], num_samples_first);
            math::copy(&dst[ch][0], &src[ch][num_samples_first], overflow);
        }
    }
    else
    {
        for (auto ch=0; ch<dst.num_channels(); ++ch)
        {
            math::copy(&dst[ch][i0], &src[ch][0], num_samples_to_add);
        }
    }
}

template <typename RingBufferType, typename BufferType>
void ring_buffer_copy_to_buffer(BufferType&& dst, const RingBufferType& src, int offset)
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
            math::copy(&dst[ch][0], &src.channel(ch)[i0], num_samples_first);
            math::copy(&dst[ch][num_samples_first], &src.channel(ch)[0], overflow);
        }
    }
    else
    {
        for (auto ch=0; ch < dst.num_channels(); ++ch)
        {
            math::copy(&dst[ch][0], &src.channel(ch)[i0], num_samples_to_copy);
        }
    }
}
    
template <typename RingBufferType, typename BufferType>
void ring_buffer_add_to_buffer(BufferType& dst, const RingBufferType& src, int offset)
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
            math::add(&dst[ch][0], &src[ch][i0], num_samples_first);
            math::add(&dst[ch][num_samples_first], &src[ch][0], overflow);
        }
    }
    else
    {
        for (auto ch=0; ch < dst.num_channels(); ++ch)
        {
            math::add(&dst[ch][0], &src[ch][i0], num_samples_to_copy);
        }
    }
}


} // namespace puro
