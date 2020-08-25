#pragma once

namespace puro {


template <typename BufferType>
void constant_fill(BufferType buffer, typename BufferType::value_type value) noexcept
{
    for (int ch = 0; ch < buffer.num_channels(); ++ch)
    {
        math::set(buffer[ch], value, buffer.length());
    }
}
    
template <typename BufferType>
void impulse_fill(BufferType buffer, int index) noexcept
{
    errorif(index < 0 || index > buffer.length(), "index out of bounds");
    for (int ch = 0; ch < buffer.num_channels(); ++ch)
    {
        math::clear(buffer[ch], buffer.length());
        buffer[ch][index] = 1;
    }
}
    
/// norm_freq between [0, 0.5)
template <typename BufferType>
void osc(BufferType buffer, typename BufferType::value_type norm_freq) noexcept
{
    for (auto ch=0; ch < buffer.num_channels(); ++ch)
    {
        math::osc(buffer[ch], norm_freq, buffer.length());
    }
}

template <typename BufferType>
void noise_fill(BufferType buffer) noexcept
{
    using FloatType = typename BufferType::value_type;

    for (int ch = 0; ch < buffer.num_channels(); ++ch)
    {
        auto dst = buffer[ch];
        for (int i = 0; i < buffer.length(); ++i)
        {
            const FloatType coef = static_cast<FloatType> (1) / static_cast<FloatType> (RAND_MAX / 2);
            const FloatType r = static_cast<FloatType> (std::rand()) * coef - 1;

            dst[i] = r;
        }
    }
}

template <typename BufferType>
void linspace_fill(BufferType buffer, typename BufferType::value_type start, typename BufferType::value_type end) noexcept
{
    using ValueType = typename BufferType::value_type;
    
    const ValueType increment = (end - start) / buffer.length();
    
    auto* ch0 = buffer[0];
    for (int i=0; i<buffer.length(); ++i)
    {
        ch0[i] = start;
        start += increment;
    }

    for (int ch = 1; ch < buffer.num_channels(); ++ch)
    {
        math::copy(buffer[ch], ch0, buffer.length());
    }
}
    
template <typename BufferType, typename KernelType>
void convolve_sparse(BufferType dst, BufferType src, KernelType kernel, int kernel_offset, int stride) noexcept
{
    buffer_clear(dst);
    
    const int kernel_pre = -kernel_offset;
    const int kernel_post = kernel.length() - kernel_offset;
    
    for (auto ch=0; ch<dst.num_channels(); ++ch)
    {
        int read_index = 0;
        int write_index = 0;
        while (read_index < src.length())
        {
            int d0 = write_index + kernel_pre;
            const int k0 = d0 < 0 ? -d0 : 0;
            d0 = math::max(d0, 0);

            int d1 = write_index + kernel_post;
            const int k1 = d1 > dst.length() ? kernel.length() - (d1 - dst.length()) : kernel.length();
            d1 = math::min(d1, dst.length());
            
            //std::cout << read_index << ":\tto (" << d0 << ", " << d1 << ") from (" << k0 << ", " << k1 << ")\n";

            math::multiply_add(dst(d0, d1)[ch], kernel(k0, k1)[ch], src[ch][read_index], k1 - k0);

            read_index += 1;
            write_index += stride;
        }
    }
}
    

/** Naive buffer fill from another buffer */
template <typename BufferType, typename SorceBufferType>
std::tuple<BufferType, int> buffer_fill(BufferType buffer, SorceBufferType source, int position) noexcept
{
    using FloatType = typename BufferType::value_type;

    // source buffer will run out, trim the destination buffer
    if (source.length() < (position + buffer.length()))
    {
        buffer = buffer_trim_length(buffer, source.length() - position);
    }

    // identical channel config
    if (buffer.num_channels() == source.num_channels())
    {
        for (int ch = 0; ch < buffer.num_channels(); ++ch)
        {
            math::copy<FloatType>(buffer.channel(ch), &source.channel(ch)[position], buffer.length());
        }
    }
    // mono source, use for all channels
    else if (source.num_channels() == 1)
    {
        for (int ch = 0; ch < buffer.num_channels(); ++ch)
        {
            math::copy(buffer.channel(ch), &source.channel(0)[position], buffer.length());
        }
    }
    else
    {
        errorif(true, "channel config combination not implemented");
    }
    
    position += buffer.length();

    return std::make_tuple(buffer, position);
}
    
/** Fill from another buffer, without resizing. Returned buffer has always same size as the called one, padded to zero instead. */
template <typename BufferType, typename SorceBufferType>
int buffer_fill_with_padding(BufferType buffer, SorceBufferType source, int readIndex) noexcept
{
    using FloatType = typename BufferType::value_type;
    
    // if position is negative, fill the beginning with zeros and crop buffer
    if (readIndex < 0)
    {
        BufferType prePad;
        std::tie(prePad, buffer) = buffer_split(buffer, -readIndex);
        constant_fill(prePad, 0);
        readIndex = 0;
    }

    // source buffer will run out, pad the end
    if (source.length() < (readIndex + buffer.length()))
    {
        BufferType postPad;
        std::tie(buffer, postPad) = buffer_split(buffer, source.length() - readIndex);
        constant_fill(postPad, 0);
    }

    // identical channel config
    if (buffer.num_channels() == source.num_channels())
    {
        for (int ch = 0; ch < buffer.num_channels(); ++ch)
        {
            math::copy(buffer.channel(ch), &source.channel(ch)[readIndex], buffer.length());
        }
    }
    // mono source, use for all channels
    else if (source.num_channels() == 1)
    {
        for (int ch = 0; ch < buffer.num_channels(); ++ch)
        {
            math::copy(buffer.channel(ch), &source.channel(0)[readIndex], buffer.length());
        }
    }
    else
    {
        errorif(true, "channel config combination not implemented");
    }

    readIndex += buffer.length();

    return readIndex;
}

} // namespace puro

