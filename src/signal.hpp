#pragma once

namespace puro {


template <typename BufferType>
void constant_fill(BufferType buffer, typename BufferType::value_type value) noexcept
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        math::set(buffer.channel(ch), value, buffer.length());
    }
}

template <typename BufferType>
void noise_fill(BufferType buffer) noexcept
{
    using FloatType = typename BufferType::value_type;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto dst = buffer.channel(ch);
        for (int i = 0; i < buffer.numSamples; ++i)
        {
            const FloatType coef = static_cast<FloatType> (1) / static_cast<FloatType> (RAND_MAX / 2);
            const FloatType r = static_cast<FloatType> (std::rand()) * coef - 1;

            dst[i] = r;
        }
    }
}

template <typename BufferType, typename ValueType>
void linspace_fill(BufferType buffer, ValueType start, const ValueType increment) noexcept
{
    auto* ch0 = buffer.channel(0);
    for (int i=0; i<buffer.length(); ++i)
    {
        ch0[i] = start;
        start += increment;
    }

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
    {
        math::copy(buffer.channel(ch), ch0, buffer.length());
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
    if (buffer.getNumChannels() == source.getNumChannels())
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            math::copy<FloatType>(buffer.channel(ch), source.channel(ch), buffer.length());
        }
        position += buffer.length();
    }
    // mono source, use for all channels
    else if (source.getNumChannels() == 1)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            math::copy(buffer.channel(ch), source.channel(0), buffer.length());
        }
        position += buffer.length();
    }
    else
    {
        errorif(true, "channel config combination not implemented");
    }

    return std::make_tuple(buffer, position);
}
    
/** Fill from another buffer, without resizing. Returned buffer has always same size as the called one, padded to zero instead. */
template <typename BufferType, typename SorceBufferType>
int buffer_fill_with_padding(BufferType buffer, SorceBufferType source, int position) noexcept
{
    using FloatType = typename BufferType::value_type;
    
    // if position is negative, fill the beginning with zeros
    if (position < 0)
    {
        BufferType prePad;
        std::tie(prePad, buffer) = buffer_split(buffer, -position);
        constant_fill(prePad, 0);
    }

    // source buffer will run out, pad the end
    if (source.length() < (position + buffer.length()))
    {
        BufferType postPad;
        std::tie(buffer, postPad) = buffer_split(buffer, source.length() - position);
        constant_fill(postPad, 0);
    }

    // identical channel config
    if (buffer.getNumChannels() == source.getNumChannels())
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            math::copy<FloatType>(buffer.channel(ch), source.channel(ch), buffer.length());
        }
        position += buffer.length();
    }
    // mono source, use for all channels
    else if (source.getNumChannels() == 1)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            math::copy(buffer.channel(ch), source.channel(0), buffer.length());
        }
        position += buffer.length();
    }
    else
    {
        errorif(true, "channel config combination not implemented");
    }

    return position;
}

} // namespace puro

