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
    for (int i=0; i<buffer.length(); ++i)
    {
        buffer(0, i) = start;
        start += increment;
    }

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
    {
        math::copy(buffer.channel(ch), buffer.channel(0), buffer.length());
    }
}

template <typename BufferType, typename SrcBufferType, typename SeqType>
std::tuple<BufferType, SeqType> buffer_fill(BufferType buffer, SrcBufferType source, SeqType seq) noexcept
{
    using FloatType = typename BufferType::value_type;

    // source buffer will run out, trim the destination buffer
    if (source.length() < (seq + buffer.length()))
    {
        buffer = trimmed_length(buffer, source.length() - seq);
    }

    // identical channel config
    if (buffer.getNumChannels() == source.getNumChannels())
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            math::copy<FloatType>(buffer.channel(ch), source.channel(ch), buffer.length());
        }
        seq += buffer.length();
    }
    // mono source, use for all channels
    else if (source.getNumChannels() == 1)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            math::copy(buffer.channel(ch), source.channel(0), buffer.length());
        }
        seq += buffer.length();
    }
    else
    {
        errorif(true, "channel config combination not implemented");
    }

    return std::make_tuple(buffer, seq);
}

} // namespace puro

