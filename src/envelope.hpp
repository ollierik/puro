#pragma once

namespace puro {


template <typename PositionType>
PositionType envelope_halfcos_get_increment(int lengthInSamples) noexcept
{
    const FloatType val = math::pi<FloatType>() / static_cast<FloatType>(lengthInSamples + 1);
    return val;
}

template <typename BufferType, typename PositionType>
PositionType envelope_halfcos_fill(BufferType buffer, PositionType position, const PositionType increment) noexcept
{
    auto dst = buffer.channel(0);
    for (int i = 0; i < buffer.length(); ++i)
    {
        dst[i] = position;
        position += increment;
    }

    math::sin(dst, buffer.length());

    // copy to other channels
    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
    {
        math::copy(buffer.channel(ch), buffer.channel(0), buffer.length());
    }

    return position;
}

template <typename PositionType>
PositionType envelope_hann_get_increment(int lengthInSamples, bool symmetric = true) noexcept
{
    const PositionType div = static_cast<PositionType>(lengthInSamples) + (symmetric ? 1 : 0);
    const PositionType val = 2 * math::pi<PositionType>() / div;
    return val;
}

template <typename BufferType, typename SeqType>
SeqType envelope_hann_fill(BufferType buffer, SeqType seq) noexcept
{
    for (int i = 0; i < buffer.length(); ++i)
    {
        const auto sample = (1 - cos(seq++)) / 2;
        buffer(0, i) = sample;
    }

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
    {
        math::copy(buffer.channel(ch), &buffer.channel(ch), buffer.length());
    }

    return seq;
}

} // namespace puro

